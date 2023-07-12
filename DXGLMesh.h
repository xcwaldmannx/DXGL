#pragma once

#include <string>
#include <vector>

#include "DXGLRenderer.h"
#include "Vertex.h"
#include "Vec3f.h"

namespace dxgl {

	struct MeshMaterialSlot {
		int startIndex = 0;
		int indexCount = 0;
		std::string material = "";
	};

	class DXGLMesh {
	public:
		DXGLMesh(const std::string& filepath);
		~DXGLMesh();

		const SP_DXGLVertexBuffer& getVertexBuffer();
		const SP_DXGLIndexBuffer& getIndexBuffer();

		std::vector<Vertex> getVertices();
		std::vector<unsigned int> getIndices();
		std::vector<MeshMaterialSlot> getMaterials();

		const Vec3f& getAABBMin();
		const Vec3f& getAABBMax();

	private:
		void computeTangents();
		void computeAxialMinAndMax(const Vec3f& vec);
		void computeAABB();

	private:
		std::vector<Vertex> m_vertices;
		std::vector<unsigned int> m_indices;
		std::vector<MeshMaterialSlot> m_materialSlots;

		Vec3f m_axialMin = { 0, 0, 0 };
		Vec3f m_axialMax = { 0, 0, 0 };

		SP_DXGLVertexBuffer m_vertexBuffer = nullptr;
		SP_DXGLIndexBuffer m_indexBuffer = nullptr;
	};

}
