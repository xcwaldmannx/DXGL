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

	PxTolerancesScale tolerance;
	tolerance.length = 1.0f;
	tolerance.speed = 9.81f;
	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, tolerance, true, m_pvd);
	PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, 0.0f, 0.0f);
	sceneDesc.cpuDispatcher = m_dispatcher;
	sceneDesc.filterShader = testCCDFilterShader;
	sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
	m_scene = m_physics->createScene(sceneDesc);

	PxPvdSceneClient* client = m_scene->getScenePvdClient();
	if (client) {
		std::cout << "PVD Active.\n";
		client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	DXGLMain::entities()->onAddComponent<RigidBodyComponent>([&](governor::EntityId id) {
		auto& transform = DXGLMain::entities()->getEntityComponent<TransformComponent>(id);
		auto& mesh = DXGLMain::entities()->getEntityComponent<MeshComponent>(id);
		auto& rigidbody = DXGLMain::entities()->getEntityComponent<RigidBodyComponent>(id);

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

		// generate shapes
		if (rigidbody.isStatic) {
			PxTriangleMeshDesc meshDesc;
			meshDesc.points.count = vertices.size() / 3;
			meshDesc.points.stride = sizeof(float) * 3;
			meshDesc.points.data = &vertices[0];

			meshDesc.triangles.count = indices.size() / 3;
			meshDesc.triangles.stride = sizeof(unsigned int) * 3;
			meshDesc.triangles.data = &indices[0];

			PxDefaultMemoryOutputStream writeBuffer;
			PxTriangleMeshCookingResult::Enum result;
			bool status = PxCookTriangleMesh(cookingParams, meshDesc, writeBuffer, &result);

			PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
			PxTriangleMesh* triangleMesh = m_physics->createTriangleMesh(readBuffer);

			PxShape* shape = m_physics->createShape(PxTriangleMeshGeometry(triangleMesh, scale), *material, true);

			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);

			PxRigidStatic* rb = PxCreateStatic(*m_physics, xTransform, *shape);
			m_scene->addActor(*rb);
			m_actors[id] = rb;
		} else {
			PxConvexMeshDesc convexDesc;
			convexDesc.points.count = vertices.size() / 3;
			convexDesc.points.stride = sizeof(Vec3f);
			convexDesc.points.data = &vertices[0];
			convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

			PxDefaultMemoryOutputStream writeBuf;
			PxCookConvexMesh(cookingParams, convexDesc, writeBuf);

			PxDefaultMemoryInputData readBuf(writeBuf.getData(), writeBuf.getSize());
			PxConvexMesh* convexMesh = m_physics->createConvexMesh(readBuf);

			PxShape* shape = m_physics->createShape(PxConvexMeshGeometry(convexMesh, scale), *material, true);

			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);

			PxRigidDynamic* rb = PxCreateDynamic(*m_physics, xTransform, *shape, rigidbody.mass);
			rb->setLinearVelocity(PxVec3(rigidbody.linearVelocity.x, rigidbody.linearVelocity.y, rigidbody.linearVelocity.z));
			rb->setAngularVelocity(PxVec3(rigidbody.angularVelocity.x, rigidbody.angularVelocity.y, rigidbody.angularVelocity.z));
			rb->setLinearDamping(0.1f);
			rb->setAngularDamping(0.1f);
			rb->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
			m_scene->addActor(*rb);
			m_actors[id] = rb;
		}

	});

	DXGLMain::entities()->onRemoveComponent<RigidBodyComponent>([&](governor::EntityId id) {
		PxActor& actor = *m_actors[id];
		m_scene->removeActor(actor);
		m_actors[id]->release();
		m_actors[id] = nullptr;
	});

	DXGLMain::entities()->onDestroyEntity([&](governor::EntityId id) {
		if (DXGLMain::entities()->entityHasComponent<RigidBodyComponent>(id)) {
			PxActor& actor = *m_actors[id];
			m_scene->removeActor(actor);
			m_actors[id]->release();
			m_actors[id] = nullptr;
		}
	});
}

PhysicsManager::~PhysicsManager() {
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
			auto& transform = DXGLMain::entities()->getEntityComponent<TransformComponent>(id);

			PxTransform pxTransform = actor->getGlobalPose();
			PxVec3& p = pxTransform.p;
			PxQuat& q = pxTransform.q;

			transform.translation.x = p.x;
			transform.translation.y = p.y;
			transform.translation.z = p.z;
			DXGLMain::entities()->relocateEntity(id);

			quatToEuler(q, transform.rotation.x, transform.rotation.y, transform.rotation.z);
		}
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
