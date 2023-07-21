#pragma once

#include "Math.h"

#include "DXGLBasicMesh.h"

namespace dxgl {

	struct TerrainFace {
		Vec3f v0;
		Vec3f v1;
		Vec3f v2;
		Vec3f center;
		Vec3f normal;
	};

	struct TerrainChunk {
		Vec3f minVertex{};
		Vec3f maxVertex{};
		std::vector<unsigned int> faceIndices{};
		std::vector<TerrainFace*> faces{};
		int indexCount = 0;
	};

	struct TerrainBuffer {
		Mat4f world{};
		Mat4f view{};
		Mat4f proj{};

		unsigned int materialFlags = 0;
		float pad[3];
	};

	struct MeshDesc;

	class DXGLTerrainManager {
	public:
		DXGLTerrainManager();
		~DXGLTerrainManager();

		void load(const MeshDesc& desc, const std::string& filename);
		void update(long double delta);
		void draw();

		float getTerrainHeight(float x, float z);

	private:

		float chunkSize = 0;

		SP_DXGLBasicMesh m_mesh = nullptr;
		SP_DXGLIndexBuffer m_ib = nullptr;

		SP_DXGLInputLayout m_layout = nullptr;

		SP_DXGLVertexShader m_vs = nullptr;
		SP_DXGLPixelShader m_ps = nullptr;

		SP_DXGLCBuffer m_cb = nullptr;

		std::vector<TerrainChunk*> m_chunks{};
		std::vector<unsigned int> m_indices{};

	};
}