#include "PhysicsManager.h"

#include "Engine.h"
#include "EntityManager.h"

#include "Mesh.h"

using namespace dxgl;
using namespace physx;

PhysicsManager::PhysicsManager() {
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);

	m_pvd = PxCreatePvd(*m_foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 1000);
	bool connected = m_pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	std::cout << "Pvd Connection: " << connected << "\n";

	m_dispatcher = PxDefaultCpuDispatcherCreate(4);

	PxCudaContextManagerDesc cudaContextManagerDesc;
	m_cudaContextManager = PxCreateCudaContextManager(*m_foundation, cudaContextManagerDesc, PxGetProfilerCallback());

	PxTolerancesScale tolerance;
	tolerance.length = 1.0f;
	tolerance.speed = 9.81f;
	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, tolerance, true, m_pvd);
	PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_dispatcher;
	sceneDesc.cudaContextManager = m_cudaContextManager;
	sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
	sceneDesc.filterShader = testCCDFilterShader;
	sceneDesc.flags |= PxSceneFlag::eENABLE_CCD | PxSceneFlag::eENABLE_GPU_DYNAMICS;;
	m_scene = m_physics->createScene(sceneDesc);

	PxPvdSceneClient* client = m_scene->getScenePvdClient();
	if (client) {
		std::cout << "PVD Active.\n";
		client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	Engine::entities()->onAddComponent<RigidBodyComponent>([&](governor::EntityId id) {
		auto& transform = Engine::entities()->getEntityComponent<TransformComponent>(id);
		auto& mesh = Engine::entities()->getEntityComponent<MeshComponent>(id);
		auto& rigidbody = Engine::entities()->getEntityComponent<RigidBodyComponent>(id);

		// transform data
		PxTransform xTransform(PxVec3(transform.translation.x, transform.translation.y, transform.translation.z));

		xTransform.p = { transform.translation.x, transform.translation.y, transform.translation.z };
		PxQuat rotationQuat = PxQuat(transform.rotation.x, PxVec3(1.0f, 0.0f, 0.0f))
			* PxQuat(transform.rotation.y, PxVec3(0.0f, 1.0f, 0.0f))
			* PxQuat(transform.rotation.z, PxVec3(0.0f, 0.0f, 1.0f));
		xTransform.q = rotationQuat;


		// mesh data
		std::vector<float>& vertices = mesh.mesh->getVertexPositions();
		std::vector<unsigned int>& indices = mesh.mesh->getVertexIndices();
		PxMeshScale scale(PxVec3(transform.scale.x, transform.scale.y, transform.scale.z), PxQuat(PxIdentity));

		// cooking data
		PxCookingParams cookingParams(m_physics->getTolerancesScale());

		// material data
		PxMaterial* material = m_physics->createMaterial(rigidbody.staticFriction, rigidbody.dynamicFriction, rigidbody.restitution);

		// num actors
		PxActorTypeFlags actorTypes = PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC;
		PxU32 numActors = m_scene->getNbActors(actorTypes);


		// generate shape

		PxShape* shape = nullptr;
		PxGeometry* geometry = nullptr;

		std::vector<PxGeometry*> geometries{};

		if (RigidBodyBox* box = dynamic_cast<RigidBodyBox*>(rigidbody.shape)) {

			geometry = new PxBoxGeometry(box->halfExtents.x, box->halfExtents.y, box->halfExtents.z);

		} else if (RigidBodySphere* sphere = dynamic_cast<RigidBodySphere*>(rigidbody.shape)) {

			geometry = new PxSphereGeometry(sphere->radius);

		} else if (RigidBodyCylinder* cylinder = dynamic_cast<RigidBodyCylinder*>(rigidbody.shape)) {

			generateCylinder(cylinder->segmentCount, cylinder->radius, cylinder->halfHeight);
			PxConvexMeshDesc convexDesc;
			convexDesc.points.count = m_cylinderVertices.size();
			convexDesc.points.stride = sizeof(Vec3f);
			convexDesc.points.data = &m_cylinderVertices[0];
			convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

			PxDefaultMemoryOutputStream writeBuf;
			PxCookConvexMesh(cookingParams, convexDesc, writeBuf);

			PxDefaultMemoryInputData readBuf(writeBuf.getData(), writeBuf.getSize());
			PxConvexMesh* convexMesh = m_physics->createConvexMesh(readBuf);

			geometry = new PxConvexMeshGeometry(convexMesh);

		} else if (RigidBodyCapsule* capsule = dynamic_cast<RigidBodyCapsule*>(rigidbody.shape)) {

			geometry = new PxCapsuleGeometry(capsule->radius, capsule->halfHeight);

		} else if (RigidBodyConvexMesh* convex = dynamic_cast<RigidBodyConvexMesh*>(rigidbody.shape)) {

			PxConvexMeshDesc convexDesc;
			convexDesc.points.count = vertices.size() / 3;
			convexDesc.points.stride = sizeof(Vec3f);
			convexDesc.points.data = &vertices[0];
			convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

			PxDefaultMemoryOutputStream writeBuf;
			PxCookConvexMesh(cookingParams, convexDesc, writeBuf);

			PxDefaultMemoryInputData readBuf(writeBuf.getData(), writeBuf.getSize());
			PxConvexMesh* convexMesh = m_physics->createConvexMesh(readBuf);

			geometry = new PxConvexMeshGeometry(convexMesh, scale);

		} else if (RigidBodyTriangleMesh* triangle = dynamic_cast<RigidBodyTriangleMesh*>(rigidbody.shape)) {

			for (auto& subMesh : mesh.mesh->getMeshes()) {
				int startIndex = subMesh.baseIndex;
				int startVertex = subMesh.baseVertex;
				int indexCount = subMesh.indexCount;

				PxTriangleMeshDesc meshDesc;
				meshDesc.points.count = (vertices.size() / 3) - startVertex; // Calculate based on startVertex and indexCount
				meshDesc.points.stride = sizeof(Vec3f);
				meshDesc.points.data = &vertices[startVertex * 3]; // Multiply by 3 to get the actual start position in vertices array

				meshDesc.triangles.count = indexCount / 3;
				meshDesc.triangles.stride = sizeof(unsigned int) * 3;
				meshDesc.triangles.data = &indices[startIndex];

				PxDefaultMemoryOutputStream writeBuffer;
				PxTriangleMeshCookingResult::Enum result;
				bool status = PxCookTriangleMesh(cookingParams, meshDesc, writeBuffer, &result);

				PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
				PxTriangleMesh* triangleMesh = m_physics->createTriangleMesh(readBuffer);

				geometries.push_back(new PxTriangleMeshGeometry(triangleMesh, scale));
			}
		} else {

			geometry = new PxBoxGeometry(1, 1, 1);

		}

		if (geometries.size() > 0) {

		} else {
			shape = m_physics->createShape(*geometry, *material, true);
			if (RigidBodyCapsule* capsule = dynamic_cast<RigidBodyCapsule*>(rigidbody.shape)) {
				PxTransform relativePose(PxQuat(PxHalfPi, PxVec3(0, 0, 1)));
				shape->setLocalPose(relativePose);
			}

			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		}

		// create rigidbodies

		if (rigidbody.isStatic) {
			PxRigidStatic* rb = nullptr;
			if (geometries.size() > 0) {
				rb = m_physics->createRigidStatic(xTransform);
				for (auto& g : geometries) {
					PxShape* es = PxRigidActorExt::createExclusiveShape(*rb, *g, *material);
				}
			} else {
				rb = PxCreateStatic(*m_physics, xTransform, *shape);
			}
			m_scene->addActor(*rb);
			m_actors[id] = rb;
			rigidbody.actor = rb;
		} else {
			PxRigidDynamic* rb = PxCreateDynamic(*m_physics, xTransform, *shape, rigidbody.mass);
			rb->setLinearVelocity(PxVec3(rigidbody.linearVelocity.x, rigidbody.linearVelocity.y, rigidbody.linearVelocity.z));
			rb->setAngularVelocity(PxVec3(rigidbody.angularVelocity.x, rigidbody.angularVelocity.y, rigidbody.angularVelocity.z));
			rb->setLinearDamping((PxReal) rigidbody.linearDamp);
			rb->setAngularDamping((PxReal) rigidbody.angularDamp);
			rb->setRigidDynamicLockFlags((PxRigidDynamicLockFlags) rigidbody.lockFlags);
			rb->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
			m_scene->addActor(*rb);
			m_actors[id] = rb;
			rigidbody.actor = rb;
		}

	});

	Engine::entities()->onRemoveComponent<RigidBodyComponent>([&](governor::EntityId id) {
		PxActor& actor = *m_actors[id];
		m_scene->removeActor(actor);
		m_actors[id]->release();
		m_actors[id] = nullptr;
	});

	Engine::entities()->onDestroyEntity([&](governor::EntityId id) {
		if (Engine::entities()->entityHasComponent<RigidBodyComponent>(id)) {
			PxActor& actor = *m_actors[id];
			m_scene->removeActor(actor);
			m_actors[id]->release();
			m_actors[id] = nullptr;
		}
	});
}

PhysicsManager::~PhysicsManager() {
	//if (m_scene) m_scene->release();
	//if (m_physics) m_physics->release();
	//if (m_dispatcher) m_dispatcher->release();
	//if (m_pvd) m_pvd->release();
	//if (m_foundation) m_foundation->release();
}

void PhysicsManager::update(std::list<OctTree<governor::EntityId>::ptr>& entities, long double delta) {

	if (delta > 0.0f) {
		m_scene->simulate(delta);
		m_scene->fetchResults(true);
	}

	for (auto entity : entities) {
		auto id = entity->item;
		PxRigidActor* actor = m_actors[id];

		if (actor && actor->is<PxRigidDynamic>()) {
			auto& transform = Engine::entities()->getEntityComponent<TransformComponent>(id);
			auto& rigidbody = Engine::entities()->getEntityComponent<RigidBodyComponent>(id);

			PxTransform pxTransform = actor->getGlobalPose();
			PxVec3& p = pxTransform.p;
			PxQuat& q = pxTransform.q;

			transform.translation.x = p.x;
			transform.translation.y = p.y;
			transform.translation.z = p.z;
			Engine::entities()->relocateEntity(id);

			quatToEuler(q, transform.rotation.x, transform.rotation.y, transform.rotation.z);

			float vy = ((PxRigidDynamic*)actor)->getLinearVelocity().y;
			if (abs(vy) <= 0.1f) {
				rigidbody.isStationary = true;
			}
			else {
				rigidbody.isStationary = false;
			}
		}
	}
}

void PhysicsManager::generateCylinder(float segmentCount, float radius, float halfHeight) {
	m_cylinderVertices.clear();
	float advance = 6.28318530718f / segmentCount;
	for (int i = 0; i < segmentCount; i++) {
		float deg = advance * i;
		Vec3f v0 = { sin(deg) * radius,  halfHeight, cos(deg) * radius };
		Vec3f v1 = { sin(deg) * radius, -halfHeight, cos(deg) * radius };
		m_cylinderVertices.push_back(v0);
		m_cylinderVertices.push_back(v1);
	}
}

PxFilterFlags PhysicsManager::testCCDFilterShader(
	PxFilterObjectAttributes attributes0,
	PxFilterData filterData0,
	PxFilterObjectAttributes attributes1,
	PxFilterData filterData1,
	PxPairFlags& pairFlags,
	const void* constantBlock,
	PxU32 constantBlockSize) {
	pairFlags = PxPairFlag::eSOLVE_CONTACT;
	pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
	pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;
	return PxFilterFlags();
}
