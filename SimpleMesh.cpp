#include "SimpleMesh.h"

using namespace dxgl;

/*
* The simplest form of a Mesh.
* Contains vertices, indices, and uv coords.
* Does not handle materials or animations.
*/

SimpleMesh::SimpleMesh(const std::string& filename) {
	// load the file
	unsigned int loadFlags = (aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);
	m_scene = m_importer.ReadFile(filename.c_str(), loadFlags);

	if (!m_scene) {
		std::cout << m_importer.GetErrorString() << "\n";
	}

	if (m_scene) {
		m_subMeshes.resize(m_scene->mNumMeshes);

		unsigned int vertexCount = 0;
		unsigned int indexCount = 0;

		// get meshes
		for (unsigned int i = 0; i < m_subMeshes.size(); i++) {
			const aiMesh* mesh = m_scene->mMeshes[i];
			m_subMeshes[i].indexCount = mesh->mNumFaces * 3;
			m_subMeshes[i].startVertex = vertexCount;
			m_subMeshes[i].startIndex = indexCount;

			vertexCount += m_scene->mMeshes[i]->mNumVertices;
			indexCount += m_subMeshes[i].indexCount;
		}

		// create meshes
		m_vertices.reserve(vertexCount * sizeof(SimpleVertex));
		m_indices.reserve(indexCount);

	}
}

SimpleMesh::~SimpleMesh() {

}
