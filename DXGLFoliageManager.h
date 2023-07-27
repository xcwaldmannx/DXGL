#pragma once

#include <algorithm>
#include <execution>
#include <future>
#include <chrono>

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

	struct FoliageChunk {
		uint32_t id = 0;
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
		void draw();

		void loadTerrain(const QuadTree<TerrainChunk>::list& terrain);
		void unloadTerrain(const QuadTree<TerrainChunk>::list& terrain);

	private:
		std::vector<FoliageChunk> asyncLoadTerrain(const QuadTree<TerrainChunk>::ptr& terrain);
		std::vector<FoliageChunk> generateChunks(const QuadTree<TerrainChunk>::ptr& terrain);

	private:

		std::vector<FoliageInstance> m_foliage{};
		std::vector<FoliageInstance> m_culledFoliage{};

		SP_DXGLCamera m_camera = nullptr;

		std::unordered_map<uint32_t, FoliageChunk> m_chunks{};
		std::unordered_map<uint32_t, std::future<std::vector<FoliageChunk>>> m_futures{};

		SP_DXGLInputLayout m_layout = nullptr;
		SP_DXGLVertexShader m_vs = nullptr;
		SP_DXGLPixelShader m_ps = nullptr;

		SP_DXGLBasicMesh m_mesh = nullptr;

		SP_DXGLCBuffer m_cb = nullptr;
		SP_DXGLVertexBuffer m_vbInstance = nullptr;

		float m_timePassed = 0;
	};
}