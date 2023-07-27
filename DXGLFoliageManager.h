#pragma once

#include <algorithm>
#include <execution>

#include "DXGLMain.h"

#include "Math.h"
#include "Vec3f.h"
#include "Mat4f.h"
#include "QuadTree.h"

namespace dxgl {

	struct TerrainChunk;

	struct FoliageInstance {
		Vec3f scale{};
		Vec3f rotation{};
		Vec3f translation{};
		Vec3f color0{};
		Vec3f color1{};
		Vec3f color2{};
		Vec3f color3{};
		float timeOffset = 0;
	};

	struct FoliageBuffer {
		Mat4f model{};
		Mat4f view{};
		Mat4f proj{};
		Vec3f camPos{};
		float time = 0;
	};

	const float GRASS_DENSITY = 32;
	const float GRASS_TILE_SIZE = 8;
	const float GRASS_TILE_LENGTH = 8;
	const float GRASS_TOTAL_LENGTH = GRASS_TILE_SIZE * GRASS_TILE_LENGTH;
	const float GRASS_BLADES_MAX = (GRASS_DENSITY * GRASS_DENSITY) * (GRASS_TILE_LENGTH * GRASS_TILE_LENGTH);

	struct FoliageChunk {
		Vec3f minVertex{};
		Vec3f maxVertex{};
		float LOD = 0;
		std::vector<FoliageInstance> foliage{};
	};

	class DXGLFoliageManager {
	public:
		DXGLFoliageManager();
		~DXGLFoliageManager();

		void update(long double delta);
		void loadTerrain(const QuadTree<TerrainChunk>::list& terrain);
		void unloadTerrain(const QuadTree<TerrainChunk>::list& terrain);
		void draw();
	private:
		void cull();

	private:

		std::vector<FoliageInstance> m_foliage{};
		std::vector<FoliageInstance> m_culledFoliage{};

		std::unordered_map<uint32_t, FoliageChunk> m_chunks{};

		SP_DXGLInputLayout m_layout = nullptr;
		SP_DXGLVertexShader m_vs = nullptr;
		SP_DXGLPixelShader m_ps = nullptr;

		SP_DXGLBasicMesh m_mesh = nullptr;

		SP_DXGLCBuffer m_cb = nullptr;
		SP_DXGLVertexBuffer m_vbInstance = nullptr;

		float m_timePassed = 0;
	};
}