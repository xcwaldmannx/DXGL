#pragma once

#include "DXGLBasicMesh.h"

namespace dxgl {

	struct TerrainChunk {
		float x1 = 0;
		float y1 = 0;
		float z1 = 0;
		float x2 = 0;
		float y2 = 0;
		float z2 = 0;
		std::vector<unsigned int> faceIndices{};
		int indexCount = 0;
	};

	struct TerrainBuffer {
		Mat4f world{};
		Mat4f view{};
		Mat4f proj{};

		unsigned int materialFlags = 0;
		float pad[3];
	};

	class DXGLTerrainSystem {
	public:
		DXGLTerrainSystem(const MeshDesc& desc, const std::string& filename);
		~DXGLTerrainSystem();

		void update(long double delta);
		void draw();
	private:
		SP_DXGLBasicMesh m_mesh = nullptr;
		SP_DXGLIndexBuffer m_ib = nullptr;

		SP_DXGLInputLayout m_layout = nullptr;

		SP_DXGLVertexShader m_vs = nullptr;
		SP_DXGLPixelShader m_ps = nullptr;

		SP_DXGLCBuffer m_cb = nullptr;

		std::vector<TerrainChunk> m_chunks{};
		std::vector<unsigned int> m_indices{};

	};
}