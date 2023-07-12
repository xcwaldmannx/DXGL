#pragma once

#include <functional>

#include "DXGLDefinitions.h"

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
	SP_DXGLMesh mesh = nullptr;
	SP_DXGLBasicMesh basicmesh = nullptr;
	bool useTessellation = false;
	int instanceFlags = 0;
};

struct PickableComponent {
	bool isSelected = false;
};

struct FoliageComponent {
	Vec3f position;
	float size;
};

struct VS_CBufferComponent {
	std::vector<std::pair<SP_DXGLCBuffer, int>> cbuffers{};

	void add(SP_DXGLCBuffer cbuffer, int slot) {
		cbuffers.push_back(std::pair<SP_DXGLCBuffer, int>(cbuffer, slot));
	}
};

struct HS_CBufferComponent {
	std::vector<std::pair<SP_DXGLCBuffer, int>> cbuffers{};

	void add(SP_DXGLCBuffer cbuffer, int slot) {
		cbuffers.push_back(std::pair<SP_DXGLCBuffer, int>(cbuffer, slot));
	}
};

struct DS_CBufferComponent {
	std::vector<std::pair<SP_DXGLCBuffer, int>> cbuffers{};

	void add(SP_DXGLCBuffer cbuffer, int slot) {
		cbuffers.push_back(std::pair<SP_DXGLCBuffer, int>(cbuffer, slot));
	}
};

struct PS_CBufferComponent {
	std::vector<std::pair<SP_DXGLCBuffer, int>> cbuffers{};

	void add(SP_DXGLCBuffer cbuffer, int slot) {
		cbuffers.push_back(std::pair<SP_DXGLCBuffer, int>(cbuffer, slot));
	}
};
