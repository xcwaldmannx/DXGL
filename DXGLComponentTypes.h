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

struct PickableComponent {
	bool isSelected = false;
};

struct DestroyableComponent {
};

struct RigidBodyComponent {
	Vec3f linearVelocity{};
	Vec3f angularVelocity{};
	float mass = 0;
	float staticFriction = 0;
	float dynamicFriction = 0;
	float restitution = 0;
	bool isStatic = false;
};

struct ControllableComponent {
public:
	float speed = 0;

	void addAction(char key, std::function<void()> function) {
		m_actions[key] = function;
	}

private:
	void executeActions(char key) {
		if (m_actions.find(key) != m_actions.end()) {
			m_actions[key]();
		}
	}

	std::unordered_map<char, std::function<void()>> m_actions{};
};

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
	bool trackMouse = true;

private:
	bool isActive = false;

	Mat4f worldMatrix{};
	Mat4f viewMatrix{};
	Mat4f projMatrix{};

	float mouseRotX = 0;
	float mouseRotY = 0;

	friend class CameraManager;
};
