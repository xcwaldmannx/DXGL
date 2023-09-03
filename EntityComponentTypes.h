#pragma once

#include <functional>

#include "PxPhysicsAPI.h"

#include "DXGLDefinitions.h"

#include "DXGLGovernor.h"

#include "Vec3f.h"
#include "Mat4f.h"

using namespace dxgl;

enum GlobalFlags {
	GLOBAL_USE_FULLSCREEN = (1 << 0),
};

enum InstanceFlags {
	INSTANCE_USE_LIGHTING  = (1 << 0),
	INSTANCE_USE_SHADOWING = (1 << 1),
	INSTANCE_USE_FOG       = (1 << 2),
	INSTANCE_IS_SELECTED   = (1 << 3),
};

struct InstanceData {
	governor::EntityId id = 0;
	Vec3f scale{};
	Vec3f rotation{};
	Vec3f translation{};
	int flags = 0;
};

struct TransformComponent {
	Vec3f scale{};
	Vec3f rotation{};
	Vec3f translation{};
};

struct MeshComponent {
	SP_Mesh mesh = nullptr;
	bool hideMesh = false;;
	bool useTessellation = false;
	int instanceFlags = 0;
};

struct PickableComponent {
	bool isSelected = false;
};

struct DestroyableComponent {
};

//////////////////////
//   Description    //
//////////////////////

struct DescriptionComponent {
	std::string name{};
	std::string description{};
};

//////////////////////
//    Rigidbody     //
//////////////////////

struct RigidBodyShape {
	virtual ~RigidBodyShape() {}
};
struct RigidBodyBox : RigidBodyShape {
	RigidBodyBox(float x, float y, float z) : halfExtents(x, y, z) {
	}

	Vec3f halfExtents{ 1, 1, 1 };
};

struct RigidBodySphere : RigidBodyShape {
	RigidBodySphere(float radius) : radius(radius) {
	}
	float radius = 1.0f;
};

struct RigidBodyCylinder : RigidBodyShape {
	RigidBodyCylinder(float segmentCount, float radius, float halfHeight) :
	segmentCount(segmentCount), radius(radius), halfHeight(halfHeight) {
	}

	float segmentCount = 16.0f;
	float radius = 1.0f;
	float halfHeight = 1.0f;
};

struct RigidBodyCapsule : RigidBodyShape {
	RigidBodyCapsule(float radius, float halfHeight) : radius(radius), halfHeight(halfHeight) {
	}

	float radius = 1.0f;
	float halfHeight = 1.0f;
};

struct RigidBodyConvexMesh : RigidBodyShape {};

struct RigidBodyTriangleMesh : RigidBodyShape {};

struct RigidBodyLockFlag {
	enum Flag {
		LOCK_LINEAR_X = physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X,
		LOCK_LINEAR_Y = physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y,
		LOCK_LINEAR_Z = physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z,
		LOCK_ANGULAR_X = physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X,
		LOCK_ANGULAR_Y = physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y,
		LOCK_ANGULAR_Z = physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z,
	};
};

struct RigidBodyComponent {
public:
	void applyTransform(Vec3f force) {
		if (actor->is<physx::PxRigidDynamic>()) {
			float y = ((physx::PxRigidDynamic*)actor)->getLinearVelocity().y;
			physx::PxTransform transform = ((physx::PxRigidDynamic*)actor)->getGlobalPose();
			transform.p += physx::PxVec3(force.x, force.y, force.z);
			((physx::PxRigidDynamic*)actor)->setGlobalPose(transform);
		}
	}

	void applyVelocity(Vec3f force) {
		if (actor->is<physx::PxRigidDynamic>()) {
			float y = ((physx::PxRigidDynamic*)actor)->getLinearVelocity().y;
			physx::PxVec3 v0 = ((physx::PxRigidDynamic*)actor)->getLinearVelocity();
			physx::PxVec3 v1 = v0 + physx::PxVec3(force.x, force.y, force.z);
			((physx::PxRigidDynamic*)actor)->setLinearVelocity(v1);
		}
	}

	RigidBodyShape* shape = nullptr;

	Vec3f linearVelocity{};
	Vec3f angularVelocity{};
	float linearDamp = 0.1f;
	float angularDamp = 0.1f;
	unsigned int lockFlags;

	float mass = 0;
	float staticFriction = 0;
	float dynamicFriction = 0;
	float restitution = 0;

	bool isStatic = false;
	bool isStationary = false;


private:
	physx::PxRigidActor* actor = nullptr;

	friend class PhysicsManager;
};

//////////////////////
//     Movement     //
//////////////////////

struct MovementComponent {
	float speed = 1.0f;
	Vec3f direction{};
};

//////////////////////
//    Controller    //
//////////////////////

struct ControllerActionFlag {
	enum Flag {
		ON_PRESS,
		ON_HOLD,
		ON_TAP,
		ON_RELEASE,
	};
};

struct ControllerComponent {
public:
	float speed = 0;
	bool isActive = false;

	void addAction(char key, ControllerActionFlag::Flag flag, std::function<void(governor::EntityId, long double)> function) {
		actions.emplace_back(ControllerAction{ key, flag, function });
	}

private:
	struct ControllerAction {
		char key;
		ControllerActionFlag::Flag flag;
		std::function<void(governor::EntityId, long double)> function;
	};

	std::vector<ControllerAction> actions{};

	friend class ControllerManager;
};

//////////////////////
//      Camera      //
//////////////////////

struct CameraComponent {
public:
	Vec3f forward() {
		return worldMatrix.getZDirection();
	}

	Vec3f right() {
		return worldMatrix.getXDirection();
	}

	Vec3f up() {
		return worldMatrix.getYDirection();
	}

	Mat4f& world() {
		return worldMatrix;
	}

	Mat4f& view() {
		return viewMatrix;
	}

	Mat4f& proj() {
		return projMatrix;
	}

	Vec3f translation{};
	Vec3f rotation{};
	bool trackMouse = false;
	bool trackEntity = false;
	bool enableThirdPerson = false;
	float thirdPersonDistance = 5.0f;

private:
	bool isActive = false;

	Mat4f worldMatrix{};
	Mat4f viewMatrix{};
	Mat4f projMatrix{};

	float mouseRotX = 0;
	float mouseRotY = 0;

	friend class CameraManager;
};
