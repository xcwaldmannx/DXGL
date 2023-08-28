#pragma once

#include <functional>

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
	bool useTessellation = false;
	int instanceFlags = 0;
};

struct RenderPassComponent {
	bool useEarlyZ = false;
	bool useShadows = false;
	bool useBloom = false;
	bool useTransparency = false;
};

struct PickableComponent {
	bool isSelected = false;
};

struct DestroyableComponent {
};

struct RigidBodyComponent {
	Vec3f linearVelocity{};
	Vec3f angularVelocity{};
	Vec3f force{};
	Vec3f torque{};
	float mass = 0;
	float friction = 0;
	float elasticity = 0;
	float moi = 0;
	bool isColliding = false;
};
