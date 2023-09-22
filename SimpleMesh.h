#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Vec2f.h"
#include "Vec3f.h"

namespace dxgl {
	struct SimpleVertex {
		Vec3f position;
		Vec2f texcoord;
		Vec3f normal;
		Vec3f tangent;
	};

	struct SimpleSubMesh {
		unsigned int indexCount;
		unsigned int startVertex;
		unsigned int startIndex;
	};

	class SimpleMesh {
	public:
		SimpleMesh(const std::string& filename);
		~SimpleMesh();

	private:
		Assimp::Importer m_importer;
		const aiScene* m_scene;

		std::vector<SimpleSubMesh> m_subMeshes{};
		std::vector<SimpleVertex> m_vertices{};
		std::vector<unsigned int> m_indices{};
	};
}
