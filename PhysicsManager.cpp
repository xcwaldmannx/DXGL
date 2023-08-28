#include "PhysicsManager.h"

using namespace dxgl;
using namespace physx;

PhysicsManager::PhysicsManager() {
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);

	m_pvd = PxCreatePvd(*m_foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 1000);
	bool connected = m_pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	std::cout << "Pvd Connection: " << connected << "\n";

	m_dispatcher = PxDefaultCpuDispatcherCreate(2);

	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, PxTolerancesScale(), true, m_pvd);

	PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	m_scene = m_physics->createScene(sceneDesc);

	PxPvdSceneClient* client = m_scene->getScenePvdClient();
	if (client) {
		std::cout << "Setup client.\n";
		client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	m_material = m_physics->createMaterial(0.5f, 0.5f, 0.5f); // Static friction, dynamic friction, restitution

	for (int i = 0; i < MAX_ENTITIES; ++i) {
		m_idToActor[i] = -1;
	}
}

PhysicsManager::~PhysicsManager() {
}

void PhysicsManager::update(std::list<OctTree<governor::EntityId>::ptr>& entities, long double delta) {
	std::vector<governor::EntityId> search{};
	for (auto entity : entities) {
		auto id = entity->item;
		if (DXGLMain::entities()->entityHasComponent<RigidBodyComponent>(id)) {
			search.push_back(id);
		}
	}

	for (auto id : search) {
		PxActorTypeFlags actorTypes = PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC;
		PxU32 numActors = m_scene->getNbActors(actorTypes);

		if (m_idToActor[id] == -1) {
			auto& transform = DXGLMain::entities()->getEntityComponent<TransformComponent>(id);
			auto& mesh = DXGLMain::entities()->getEntityComponent<MeshComponent>(id);
			auto& rigidbody = DXGLMain::entities()->getEntityComponent<RigidBodyComponent>(id);

			PxTransform xTransform(PxVec3(transform.translation.x, transform.translation.y, transform.translation.z));

			xTransform.p = { transform.translation.x, transform.translation.y, transform.translation.z };
			PxQuat rotationQuat = PxQuat(transform.rotation.x, PxVec3(1.0f, 0.0f, 0.0f))
				* PxQuat(transform.rotation.y, PxVec3(0.0f, 1.0f, 0.0f))
				* PxQuat(transform.rotation.z, PxVec3(0.0f, 0.0f, 1.0f));
			xTransform.q = rotationQuat;

			PxShape* shape = nullptr;

			// create convex hull start
			
			std::vector<float>& vertices = mesh.mesh->getVertexPositions();
			std::vector<unsigned int>& indices = mesh.mesh->getVertexIndices();

			if (rigidbody.mass > 0.0f) {
				PxConvexMeshDesc convexDesc;
				convexDesc.points.count = vertices.size() / 3;
				convexDesc.points.stride = sizeof(Vec3f);
				convexDesc.points.data = &vertices[0];
				convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

				PxCookingParams cookingParams(m_physics->getTolerancesScale());

				PxDefaultMemoryOutputStream writeBuf;
				PxCookConvexMesh(cookingParams, convexDesc, writeBuf);

				PxDefaultMemoryInputData readBuf(writeBuf.getData(), writeBuf.getSize());
				PxConvexMesh* convexMesh = m_physics->createConvexMesh(readBuf);

				PxMeshScale scale(PxVec3(transform.scale.x, transform.scale.y, transform.scale.z), PxQuat(PxIdentity));
				shape = m_physics->createShape(PxConvexMeshGeometry(convexMesh, scale), *m_material);
			} else {
				PxTriangleMeshDesc meshDesc;
				meshDesc.points.count = vertices.size() / 3;
				meshDesc.points.stride = sizeof(float) * 3;
				meshDesc.points.data = &vertices[0];

				meshDesc.triangles.count = indices.size() / 3;
				meshDesc.triangles.stride = sizeof(unsigned int) * 3;
				meshDesc.triangles.data = &indices[0];

				PxCookingParams params(m_physics->getTolerancesScale());

				PxDefaultMemoryOutputStream writeBuffer;
				PxTriangleMeshCookingResult::Enum result;
				bool status = PxCookTriangleMesh(params, meshDesc, writeBuffer, &result);
				if (!status) {
					std::cout << "FAILED\n";
				}

				PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
				PxTriangleMesh* triangleMesh = m_physics->createTriangleMesh(readBuffer);

				PxMeshScale scale(PxVec3(transform.scale.x, transform.scale.y, transform.scale.z), PxQuat(PxIdentity));
				shape = m_physics->createShape(PxTriangleMeshGeometry(triangleMesh, scale), *m_material);
			}

			// create convex hull end

			if (rigidbody.mass > 0.0f) {
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
				shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);

				PxRigidDynamic* rb = PxCreateDynamic(*m_physics, xTransform, *shape, rigidbody.mass);
				rb->setLinearVelocity(PxVec3(rigidbody.linearVelocity.x, rigidbody.linearVelocity.y, rigidbody.linearVelocity.z));
				m_scene->addActor(*rb);
				m_actors[id] = rb;
			} else {
				shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);

				PxRigidStatic* rb = PxCreateStatic(*m_physics, xTransform, *shape);
				m_scene->addActor(*rb);
				m_actors[id] = rb;
			}

			m_idToActor[id] = numActors;
		}
	}
	if (delta > 0) {
		m_scene->simulate(delta);
		m_scene->fetchResults(true);
	}

	for (auto id : search) {
		PxRigidActor* actor = m_actors[id];

		if (actor && actor->is<PxRigidDynamic>()) {
			auto& transform = DXGLMain::entities()->getEntityComponent<TransformComponent>(id);

			PxTransform pxTransform = actor->getGlobalPose();
			PxVec3& p = pxTransform.p;
			PxQuat& q = pxTransform.q;

			transform.translation.x = p.x;
			transform.translation.y = p.y;
			transform.translation.z = p.z;

			quatToEuler(q, transform.rotation.x, transform.rotation.y, transform.rotation.z);
		}
	}

	return;

	PxActorTypeFlags actorTypes = PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC;
	PxU32 numActors = m_scene->getNbActors(actorTypes);
	PxActor** actors = new PxActor*[numActors];
	m_scene->getActors(actorTypes, actors, numActors);

	for (auto id : search) {
		int actor = m_idToActor[id];
		PxRigidDynamic* rigidDynamic = actors[actor]->is<PxRigidDynamic>();
		// PxRigidStatic* rigidStatic = actors[actor]->is<PxRigidStatic>();

		if (rigidDynamic) {
			auto& transform = DXGLMain::entities()->getEntityComponent<TransformComponent>(id);

			PxTransform pxTransform = rigidDynamic->getGlobalPose();
			PxVec3& p = pxTransform.p;

			//if (id == 3794)
			//std::cout << "Entity " << id << ": (" << p.x << ", " << p.y << ", " << p.z << ")\n";

			transform.translation.x = p.x;
			transform.translation.y = p.y;
			transform.translation.z = p.z;
		}
	}

	return;

	detectCollision(search);
	respondCollision(m_detected, delta);

	for (auto id : search) {
		auto& transform = DXGLMain::entities()->getEntityComponent<TransformComponent>(id);
		auto& rigidbody = DXGLMain::entities()->getEntityComponent<RigidBodyComponent>(id);
		if (Vec3f::length(rigidbody.linearVelocity) >= 0.01f) {
			rigidbody.linearVelocity.y += -0.5f * rigidbody.mass * delta;
			transform.translation += rigidbody.linearVelocity * delta;
			DXGLMain::entities()->relocateEntity(id);
		}
	}
}

void PhysicsManager::detectCollision(std::vector<governor::EntityId>& entities) {
	m_detected.clear();

	for (auto id : entities) {
		auto& transform = DXGLMain::entities()->getEntityComponent<TransformComponent>(id);
		auto& mesh = DXGLMain::entities()->getEntityComponent<MeshComponent>(id);

		Mat4f mat{};
		mat.setTransform(transform.scale, transform.rotation, transform.translation);

		AABB aabb = mesh.mesh->getAABB();
		Vec3f center = {
			(aabb.min.x + aabb.max.x) * 0.5f,
			(aabb.min.y + aabb.max.y) * 0.5f,
			(aabb.min.z + aabb.max.z) * 0.5f
		};

		Vec3f extents = aabb.max;

		OOBB oobb(center, extents, mat);

		for (auto cId : entities) {
			if (id == cId) continue;

			auto& cTransform = DXGLMain::entities()->getEntityComponent<TransformComponent>(cId);
			auto& cMesh = DXGLMain::entities()->getEntityComponent<MeshComponent>(cId);


			Mat4f cMat{};
			cMat.setTransform(cTransform.scale, cTransform.rotation, cTransform.translation);

			AABB cAABB = cMesh.mesh->getAABB();
			Vec3f cCenter = {
				(cAABB.min.x + cAABB.max.x) * 0.5f,
				(cAABB.min.y + cAABB.max.y) * 0.5f,
				(cAABB.min.z + cAABB.max.z) * 0.5f
			};

			Vec3f cExtents = cAABB.max;

			OOBB cOOBB(cCenter, cExtents, cMat);

			if (oobb.isCollided(cOOBB)) {
				m_detected.push_back(id);
			}
		}
	}
}

void PhysicsManager::respondCollision(std::vector<governor::EntityId>& entities, long double delta) {
	for (auto id : entities) {
		auto& transform = DXGLMain::entities()->getEntityComponent<TransformComponent>(id);
		auto& mesh = DXGLMain::entities()->getEntityComponent<MeshComponent>(id);
		auto& rigidbody = DXGLMain::entities()->getEntityComponent<RigidBodyComponent>(id);

		if (rigidbody.mass <= 0.0f) continue;

		rigidbody.isColliding = false;

		Mat4f mat{};
		mat.setTransform(transform.scale, transform.rotation, transform.translation);

		for (auto cId : entities) {
			if (id == cId) continue;

			auto& cTransform = DXGLMain::entities()->getEntityComponent<TransformComponent>(cId);
			auto& cMesh = DXGLMain::entities()->getEntityComponent<MeshComponent>(cId);
			auto& cRigidbody = DXGLMain::entities()->getEntityComponent<RigidBodyComponent>(cId);

			Mat4f cMat{};
			cMat.setTransform(cTransform.scale, cTransform.rotation, cTransform.translation);

			Vec3f intersection{};
			bool foundIntersection = false;

			// loop through faces to determine exact contact point
			const auto& faces = mesh.mesh->getFaces();
			const auto& colliderFaces = cMesh.mesh->getFaces();

			Vec3f surfaceNormal{};
			Vec3f cSurfaceNormal{};

			for (const auto& face : faces) {
				Vec3f f0 = mat * face.v0;
				Vec3f f1 = mat * face.v1;
				Vec3f f2 = mat * face.v2;

				surfaceNormal = Vec3f::cross(f1 - f0, f2 - f0).normalize();

				for (const auto& cFace : colliderFaces) {
					Vec3f cf0 = cMat * cFace.v0;
					Vec3f cf1 = cMat * cFace.v1;
					Vec3f cf2 = cMat * cFace.v2;

					cSurfaceNormal = Vec3f::cross(cf1 - cf0, cf2 - cf0).normalize();

					if (Math::lineIntersectsTriangle(f0, f1, f2, cf0, cf1, intersection)) {
						foundIntersection = true;
						break;
					} else if (Math::lineIntersectsTriangle(f0, f1, f2, cf1, cf2, intersection)) {
						foundIntersection = true;
						break;
					} else if (Math::lineIntersectsTriangle(f0, f1, f2, cf2, cf0, intersection)) {
						foundIntersection = true;
						break;
					}
				}

				if (foundIntersection) {
					rigidbody.isColliding = true;
					break;
				}
			}

			// respond

			if (!foundIntersection) continue;

			// calculate final velocity

			Vec3f v1f{};
			Vec3f v2f{};
			Vec3f vRelative{};
			float impulseDenom = 0;

			if (cRigidbody.mass > 0.0f) {
				vRelative = rigidbody.linearVelocity - cRigidbody.linearVelocity;
				impulseDenom = 1.0f / (rigidbody.mass + cRigidbody.mass);

				v1f = rigidbody.linearVelocity - vRelative * impulseDenom * (1.0f + rigidbody.elasticity) * cRigidbody.mass;
				v2f = cRigidbody.linearVelocity + vRelative * impulseDenom * (1.0f + cRigidbody.elasticity) * rigidbody.mass;
			} else {
				vRelative = rigidbody.linearVelocity * -1.0f;
				impulseDenom = 1.0f / rigidbody.mass;

				v1f = vRelative * impulseDenom * (1.0f + rigidbody.elasticity);
			}

			// calculate penetration depth

			Vec3f contactNormal = (intersection - transform.translation).normalize();
			Vec3f locationBefore = transform.translation - rigidbody.linearVelocity * delta;
			Vec3f penetrationVector = intersection - locationBefore;
			float penetrationDepth = Vec3f::dot(penetrationVector, contactNormal);

			if (penetrationDepth > 0.0f) {
				// Move the object out of penetration along the contact normal
				transform.translation -= contactNormal * penetrationDepth * 0.5f;
			}
			std::cout << penetrationDepth << "\n";

			// apply velocity
			if (cRigidbody.mass > 0.0f) {
				rigidbody.linearVelocity = v1f;
				cRigidbody.linearVelocity = v2f;
			} else {
				rigidbody.linearVelocity = v1f;
			}
		}
	}
}
