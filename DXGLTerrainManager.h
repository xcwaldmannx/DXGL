#pragma once

#include <algorithm>
#include <execution>

#include "Math.h"
#include "QuadTree.h"

#include "Mesh.h"

namespace dxgl {

	struct TerrainFace {
		Vec3f v0;
		Vec3f v1;
		Vec3f v2;
		Vec3f normal;
		Vec3f center;
	};

	struct TerrainChunk {
		uint32_t id = 0;
		Vec3f minVertex{};
		Vec3f maxVertex{};
		std::vector<unsigned int> faceIndices{};
		std::vector<TerrainFace> faces{};
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

		SP_Mesh m_mesh = nullptr;
		SP_IndexBuffer m_ib = nullptr;

		SP_InputLayout m_layout = nullptr;

		SP_DXGLVertexShader m_vs = nullptr;
		SP_DXGLPixelShader m_ps = nullptr;

		SP_VSConstantBuffer m_vscb = nullptr;
		SP_PSConstantBuffer m_pscb = nullptr;

		std::vector<unsigned int> m_indices{};

		SP_DXGLCamera m_camera = nullptr;

		QuadTreeRect m_area{};
		QuadTreeRect m_searchArea{};
		QuadTree<TerrainChunk> m_chunkTree{m_area, 8};
		std::list<std::list<TerrainChunk>::iterator> m_lastSearch;
	};
}