#pragma once

#include "DXGLMain.h"

#include "Vec3f.h"
#include "Mat4f.h"

namespace dxgl {

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
		float time = 0;
		float pad[3];
	};

	const float GRASS_DENSITY = 32;
	const float GRASS_TILE_SIZE = 4;
	const float GRASS_TILE_LENGTH = 16;
	const float GRASS_TOTAL_LENGTH = GRASS_TILE_SIZE * GRASS_TILE_LENGTH;
	const float GRASS_BLADES_MAX = (GRASS_DENSITY * GRASS_DENSITY) * (GRASS_TILE_LENGTH * GRASS_TILE_LENGTH);

	struct FoliageChunk {
		float lastX = 0;
		float lastZ = 0;
		float curX = 0;
		float curZ = 0;
		int bufferLocation = 0;
	};

	class DXGLFoliageManager {
	public:
		DXGLFoliageManager();
		~DXGLFoliageManager();

		void generateFoliage();

		void update(long double delta);
		void draw();
	private:
		void cull();

	private:
		std::vector<FoliageInstance> m_foliage{};
		std::vector<FoliageInstance> m_culledFoliage{};

		std::vector<FoliageChunk> m_foliageChunks{};
		std::vector<int> m_visibleBufferLocations{};

		SP_DXGLInputLayout m_layout = nullptr;
		SP_DXGLVertexShader m_vs = nullptr;
		SP_DXGLPixelShader m_ps = nullptr;

		SP_DXGLBasicMesh m_mesh = nullptr;

		SP_DXGLCBuffer m_cb = nullptr;
		SP_DXGLVertexBuffer m_vbInstance = nullptr;

		float m_timePassed = 0;
	};
}