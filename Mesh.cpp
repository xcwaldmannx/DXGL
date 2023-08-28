#include "Mesh.h"

using namespace dxgl;

Mesh::Mesh(const MeshDesc& desc, const std::string& filename) : m_desc(desc) {

	Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);

	m_scene = m_importer.ReadFile(filename.c_str(), ASSIMP_LOAD_FLAGS);

	if (!m_scene) {
		std::cout << m_importer.GetErrorString() << "\n";
	}

	if (m_scene) {
		convertMatrix(m_scene->mRootNode->mTransformation, m_globalInverseTransform);
		m_globalInverseTransform.inverse();

		m_meshes.resize(m_scene->mNumMeshes);
		m_meshBaseVertex.resize(m_scene->mNumMeshes);

		// get meshes
		unsigned int vertexCount = 0;
		unsigned int indexCount = 0;

		for (unsigned int i = 0; i < m_meshes.size(); i++) {
			const aiMesh* mesh = m_scene->mMeshes[i];
			m_meshes[i].indexCount = mesh->mNumFaces * 3;
			m_meshes[i].baseVertex = vertexCount;
			m_meshes[i].baseIndex = indexCount;
			m_meshes[i].materialIndex = mesh->mMaterialIndex;

			m_meshBaseVertex[i] = vertexCount;

			vertexCount += m_scene->mMeshes[i]->mNumVertices;
			indexCount += m_meshes[i].indexCount;


			if (m_desc.miscAttributes & MISC_ANIMATION) {
				m_bones.resize(vertexCount);

				if (mesh->HasBones()) {
					loadBones(i, mesh);
				}
			}
		}

		if (m_desc.miscAttributes & MISC_ANIMATION) {
			m_vbBone = DXGLMain::resource()->createVertexBuffer(&m_bones[0], m_bones.size(), sizeof(unsigned int) * 4 + sizeof(float) * 4);
		}

		// create meshes
		unsigned int vertexSizePosition = (m_desc.vertexAttributes & VERTEX_POSITION) ? (4 * 3) : 0;
		unsigned int vertexSizeTexcoord = (m_desc.vertexAttributes & VERTEX_TEXCOORD) ? (4 * 2) : 0;
		unsigned int vertexSizeNormal   = (m_desc.vertexAttributes & VERTEX_NORMAL)   ? (4 * 3) : 0;
		unsigned int vertexSizeTangent  = (m_desc.vertexAttributes & VERTEX_TANGENT)  ? (4 * 3) : 0;
		unsigned int vertexSize = vertexSizePosition + vertexSizeTexcoord + vertexSizeNormal + vertexSizeTangent;

		m_vertices.reserve(vertexCount * vertexSize);
		m_indices.reserve(indexCount);

		for (unsigned int meshIndex = 0; meshIndex < m_meshes.size(); meshIndex++) {
			const aiMesh* mesh = m_scene->mMeshes[meshIndex];

			for (unsigned int vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++) {

				if (m_desc.vertexAttributes & VERTEX_POSITION) {
					const aiVector3D& position = mesh->mVertices[vertexIndex];
					m_positions.push_back(position.x);
					m_positions.push_back(position.y);
					m_positions.push_back(position.z);

					m_vertices.push_back(position.x);
					m_vertices.push_back(position.y);
					m_vertices.push_back(position.z);
					computeAxialMinAndMax(Vec3f{ position.x, position.y, position.z });
				}

				if (m_desc.vertexAttributes & VERTEX_TEXCOORD) {
					const aiVector3D& texcoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][vertexIndex] : aiVector3D(0, 0, 0);
					m_vertices.push_back(texcoord.x);
					m_vertices.push_back(texcoord.y);
				}

				if (m_desc.vertexAttributes & VERTEX_NORMAL) {
					const aiVector3D& normal = mesh->mNormals[vertexIndex];
					m_vertices.push_back(normal.x);
					m_vertices.push_back(normal.y);
					m_vertices.push_back(normal.z);
				}

				if (m_desc.vertexAttributes & VERTEX_TANGENT) {
					const aiVector3D& tangent = mesh->HasTangentsAndBitangents() ? mesh->mTangents[vertexIndex] : aiVector3D(0, 0, 0);
					m_vertices.push_back(tangent.x);
					m_vertices.push_back(tangent.y);
					m_vertices.push_back(tangent.z);
				}
			}

			if (m_desc.miscAttributes & MISC_INDEX) {
				for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) {
					const aiFace& face = mesh->mFaces[faceIndex];
					unsigned int i0 = face.mIndices[0];
					unsigned int i1 = face.mIndices[1];
					unsigned int i2 = face.mIndices[2];
					m_indices.push_back(i0);
					m_indices.push_back(i1);
					m_indices.push_back(i2);

					if (m_desc.miscAttributes & MISC_FACE) {

						Vec3f indices = {(float) i0, (float) i1, (float) i2};

						Vec3f v0 = Vec3f{ mesh->mVertices[i0].x, mesh->mVertices[i0].y, mesh->mVertices[i0].z };
						Vec3f v1 = Vec3f{ mesh->mVertices[i1].x, mesh->mVertices[i1].y, mesh->mVertices[i1].z };
						Vec3f v2 = Vec3f{ mesh->mVertices[i2].x, mesh->mVertices[i2].y, mesh->mVertices[i2].z };

						Face facee = Face{ indices, v0, v1, v2 };

						m_faces.push_back(facee);
					}
				}
			}
		}

		computeAABB();

		m_vbPosition = DXGLMain::resource()->createVertexBuffer(&m_positions[0], m_positions.size() / 3, sizeof(float) * 3);
		m_vbMesh = DXGLMain::resource()->createVertexBuffer(&m_vertices[0], vertexCount, vertexSize);

		if (m_desc.miscAttributes & MISC_INDEX) {
			m_ib = DXGLMain::resource()->createIndexBuffer(&m_indices[0], m_indices.size());
		}

		// get materials embedded in fbx
		if (m_desc.miscAttributes & MISC_MATERIAL) {
			loadMaterialTextures(m_scene);
		}

	} else {
		throw std::runtime_error("Model '" + filename + "' could not be loaded.");
	}
}

Mesh::~Mesh() {

}

void Mesh::loadMaterialTextures(const aiScene* scene) {
	m_materialNames.resize(scene->mNumMaterials);

	const int MAX_TEXTURE_TYPE = 5;

	unsigned int loadOrder[MAX_TEXTURE_TYPE] = {
		aiTextureType_NORMALS,
		aiTextureType_DIFFUSE,
		aiTextureType_METALNESS,
		aiTextureType_SHININESS,
		aiTextureType_SPECULAR,
	};

	for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
		const aiMaterial* material = scene->mMaterials[i];

		MaterialData m{};
		std::string textureName;

		for (unsigned int textureTypeIndex = 0; textureTypeIndex < MAX_TEXTURE_TYPE; textureTypeIndex++) {
			aiTextureType textureType = static_cast<aiTextureType>(loadOrder[textureTypeIndex]);

			unsigned int textureCount = material->GetTextureCount(textureType);
			for (unsigned int textureIndex = 0; textureIndex < textureCount; textureIndex++) {
				aiString texturePath;

				aiReturn result = material->GetTexture(textureType, textureIndex, &texturePath, 0, 0, 0, 0, 0);

				if (textureName.empty()) {
					textureName = std::string(texturePath.C_Str());
					int start = textureName.find_last_of("\\") + 1;
					int end = textureName.find_last_of("_");
					textureName = textureName.substr(start, end - start);

					std::cout << "Loading material: '" << textureName << "'...\n";
				}

				if (!DXGLMain::resource()->find<SP_Material>(textureName) && result == aiReturn_SUCCESS) {
					if (auto texture = scene->GetEmbeddedTexture(texturePath.C_Str())) {
						int width = 0;
						int height = 0;
						int channels = 0;

						unsigned char* data = nullptr;
						if (texture->mHeight == 0) {
							data = DXGLImage::loadFromMemory(reinterpret_cast<unsigned char*>(texture->pcData),
								texture->mWidth, width, height, channels);
						}
						else {
							data = DXGLImage::loadFromMemory(reinterpret_cast<unsigned char*>(texture->pcData),
								texture->mWidth * texture->mHeight, width, height, channels);
						}

						TextureData t {
							(TextureType)textureType,
							width,
							height,
							channels,
							data,
						};

						m_textureIndex = m_textureIndex | (1 << textureTypeIndex);

						m.textures.push_back(t);
					}
				}
			}
		}

		m_materialNames[i] = textureName;
		DXGLMain::resource()->storeMaterial(m, textureName);

		std::cout << "Done.\n";
	}

	// set mesh material
	for (auto& mesh : m_meshes) {
		mesh.materialName = m_materialNames[mesh.materialIndex];
	}
}

void Mesh::loadBones(unsigned int meshIndex, const aiMesh* mesh) {
	for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
		const aiBone* bone = mesh->mBones[boneIndex];

		int boneId = 0;
		std::string boneName = bone->mName.C_Str();

		if (m_boneNameToIndex.find(boneName) == m_boneNameToIndex.end()) {
			boneId = m_boneNameToIndex.size();
			m_boneNameToIndex[boneName] = boneId;
		}

		if (boneId == m_boneTransforms.size()) {
			MatrixBoneData boneTransform{};
			Mat4f mat{};
			convertMatrix(bone->mOffsetMatrix, mat);
			boneTransform.offset = mat;
			m_boneTransforms.push_back(boneTransform);
		}

		for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++) {
			const aiVertexWeight& vw = bone->mWeights[weightIndex];

			unsigned int globalVertexId = m_meshBaseVertex[meshIndex] + vw.mVertexId;
			m_bones[globalVertexId].addBoneData(boneId, vw.mWeight);
		}
	}
}

void Mesh::getBoneTransforms(unsigned int animationIndex, long double deltaTime, std::vector<Mat4f>& transforms) {
	Mat4f identity{};
	identity.setIdentity();

	if (m_scene->HasAnimations()) {

		long double ticksPerSec = (long double)m_scene->mAnimations[animationIndex]->mTicksPerSecond != 0 ? m_scene->mAnimations[animationIndex]->mTicksPerSecond : 25;
		long double timeInTicks = deltaTime * ticksPerSec;
		long double animationTimeTicks = std::fmod(timeInTicks, (long double)m_scene->mAnimations[animationIndex]->mDuration);

		readNodeHierarchy(animationIndex, animationTimeTicks, m_scene->mRootNode, identity);
	} else {
		readNodeHierarchy(animationIndex, 0, m_scene->mRootNode, identity);
	}
	transforms.resize(m_boneTransforms.size());

	for (unsigned int i = 0; i < m_boneTransforms.size(); i++) {
		transforms[i] = m_boneTransforms[i].transform;
	}
}

float Mesh::amountMetallic() {
	return m_desc.amountMetallic;
}

float Mesh::amountRoughness() {
	return m_desc.amountRoughness;
}

const AABB Mesh::getAABB() {
	return AABB{m_axialMin, m_axialMax};
}

void Mesh::readNodeHierarchy(unsigned int animationIndex, long double animationTimeTicks, const aiNode* node, Mat4f& parentTransform) {
	std::string nodeName(node->mName.data);

	const aiAnimation* animation = m_scene->HasAnimations() ? m_scene->mAnimations[animationIndex] : nullptr;

	Mat4f nodeTransform{};
	convertMatrix(node->mTransformation, nodeTransform);

	const aiNodeAnim* nodeAnim = m_scene->HasAnimations() ? findNodeAnim(animation, nodeName) : nullptr;

	if (nodeAnim) {
		aiVector3D scale{};
		calcInterpolatedScale(scale, animationTimeTicks, nodeAnim);
		aiMatrix4x4 scaleTransform{};
		aiMatrix4x4::Scaling(scale, scaleTransform);

		aiQuaternion rotation{};
		calcInterpolatedRotation(rotation, animationTimeTicks, nodeAnim);
		aiMatrix4x4 rotationTransform = aiMatrix4x4(rotation.GetMatrix());

		aiVector3D translation{};
		calcInterpolatedTranslation(translation, animationTimeTicks, nodeAnim);
		aiMatrix4x4 translationTransform{};
		aiMatrix4x4::Translation(translation, translationTransform);

		aiMatrix4x4 finalTransform = translationTransform * rotationTransform * scaleTransform;
		convertMatrix(finalTransform, nodeTransform);
	}

	Mat4f globalTransform = parentTransform * nodeTransform;

	if (m_boneNameToIndex.find(nodeName) != m_boneNameToIndex.end()) {
		unsigned int boneIndex = m_boneNameToIndex[nodeName];
		m_boneTransforms[boneIndex].transform = m_globalInverseTransform * globalTransform * m_boneTransforms[boneIndex].offset;
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		readNodeHierarchy(animationIndex, animationTimeTicks, node->mChildren[i], globalTransform);
	}
}

const aiNodeAnim* Mesh::findNodeAnim(const aiAnimation* animation, const std::string& nodeName) {
	for (unsigned int i = 0; i < animation->mNumChannels; i++) {
		const aiNodeAnim* nodeAnim = animation->mChannels[i];

		if (std::string(nodeAnim->mNodeName.data) == nodeName) {
			return nodeAnim;
		}
	}

	return nullptr;
}

void Mesh::calcInterpolatedScale(aiVector3D& scale, long double animationTimeTicks, const aiNodeAnim* nodeAnim) {
	if (nodeAnim->mNumScalingKeys == 1) {
		scale = nodeAnim->mScalingKeys[0].mValue;
		return;
	}

	unsigned int scalingIndex = 0;

	for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys - 1; i++) {
		long double t = (long double) nodeAnim->mScalingKeys[i + 1].mTime;
		if (animationTimeTicks < t) {
			scalingIndex = i;
			break;
		}
	}

	unsigned int nextScalingIndex = scalingIndex + 1;

	long double t1 = (long double) nodeAnim->mScalingKeys[scalingIndex].mTime;
	long double t2 = (long double) nodeAnim->mScalingKeys[nextScalingIndex].mTime;
	long double deltaTime = t2 - t1;
	long double factor = (animationTimeTicks - t1) / deltaTime;
	const aiVector3D& start = nodeAnim->mScalingKeys[scalingIndex].mValue;
	const aiVector3D& end = nodeAnim->mScalingKeys[nextScalingIndex].mValue;
	aiVector3D delta = end - start;
	scale = start + (float) factor * delta;
}

void Mesh::calcInterpolatedRotation(aiQuaternion& rotation, long double animationTimeTicks, const aiNodeAnim* nodeAnim) {
	if (nodeAnim->mNumRotationKeys == 1) {
		rotation = nodeAnim->mRotationKeys[0].mValue;
		return;
	}

	unsigned int rotationIndex = 0;

	for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++) {
		long double t = (long double) nodeAnim->mRotationKeys[i + 1].mTime;
		if (animationTimeTicks < t) {
			rotationIndex = i;
			break;
		}
	}

	unsigned int nextRotationIndex = rotationIndex + 1;

	long double t1 = (long double)nodeAnim->mRotationKeys[rotationIndex].mTime;
	long double t2 = (long double)nodeAnim->mRotationKeys[nextRotationIndex].mTime;
	long double deltaTime = t2 - t1;
	long double factor = (animationTimeTicks - t1) / deltaTime;
	const aiQuaternion& start = nodeAnim->mRotationKeys[rotationIndex].mValue;
	const aiQuaternion& end = nodeAnim->mRotationKeys[nextRotationIndex].mValue;
	aiQuaternion::Interpolate(rotation, start, end, factor);
	rotation = start;
	rotation.Normalize();
}

void Mesh::calcInterpolatedTranslation(aiVector3D& translation, long double animationTimeTicks, const aiNodeAnim* nodeAnim) {
	if (nodeAnim->mNumPositionKeys == 1) {
		translation = nodeAnim->mPositionKeys[0].mValue;
		return;
	}

	unsigned int translationIndex = 0;

	for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++) {
		long double t = (long double) nodeAnim->mPositionKeys[i + 1].mTime;
		if (animationTimeTicks < t) {
			translationIndex = i;
			break;
		}
	}

	unsigned int nextTranslationIndex = translationIndex + 1;

	long double t1 = (long double)nodeAnim->mPositionKeys[translationIndex].mTime;
	long double t2 = (long double)nodeAnim->mPositionKeys[nextTranslationIndex].mTime;
	long double deltaTime = t2 - t1;
	long double factor = (animationTimeTicks - t1) / deltaTime;
	const aiVector3D& start = nodeAnim->mPositionKeys[translationIndex].mValue;
	const aiVector3D& end = nodeAnim->mPositionKeys[nextTranslationIndex].mValue;
	aiVector3D delta = end - start;
	translation = start + (float) factor * delta;
}

void Mesh::computeAxialMinAndMax(const Vec3f& vec) {
	m_axialMin.x = vec.x < m_axialMin.x ? vec.x : m_axialMin.x;
	m_axialMin.y = vec.y < m_axialMin.y ? vec.y : m_axialMin.y;
	m_axialMin.z = vec.z < m_axialMin.z ? vec.z : m_axialMin.z;

	m_axialMax.x = vec.x > m_axialMax.x ? vec.x : m_axialMax.x;
	m_axialMax.y = vec.y > m_axialMax.y ? vec.y : m_axialMax.y;
	m_axialMax.z = vec.z > m_axialMax.z ? vec.z : m_axialMax.z;
}

void Mesh::computeAABB() {
	std::vector<Vec3f>verticesBB = {
		// FRONT FACE VERTICES
		// top left
		Vec3f(m_axialMin.x, m_axialMax.y, m_axialMin.z),
		// bottom left
		Vec3f(m_axialMin.x, m_axialMin.y, m_axialMin.z),
		// bottom right
		Vec3f(m_axialMax.x, m_axialMin.y, m_axialMin.z),
		// top right
		Vec3f(m_axialMax.x, m_axialMax.y, m_axialMin.z),

		// BACK FACE VERTICES
		// top left
		Vec3f(m_axialMin.x, m_axialMax.y, m_axialMax.z),
		// bottom left
		Vec3f(m_axialMin.x, m_axialMin.y, m_axialMax.z),
		// bottom right
		Vec3f(m_axialMax.x, m_axialMin.y, m_axialMax.z),
		// top right
		Vec3f(m_axialMax.x, m_axialMax.y, m_axialMax.z),
	};

	std::vector<unsigned int>indicesBB = {
		// FRONT
		0, 1,
		1, 2,
		2, 3,
		3, 0,

		// BACK
		4, 5,
		5, 6,
		6, 7,
		7, 4,

		// LEFT
		0, 4,
		4, 5,
		5, 1,
		1, 0,

		// RIGHT
		3, 7,
		7, 6,
		6, 2,
		2, 3,

		// TOP
		0, 3,
		3, 7,
		7, 4,
		4, 0,

		// BOTTOM
		1, 2,
		2, 6,
		6, 5,
		5, 1,
	};
}

float Mesh::calculateEdgeError(const std::vector<Vertex>& vertices, const Edge& edge) {
	// Get the positions of the vertices connected by the edge
	const Vec3f& position1 = vertices[edge.vIndex0].position;
	const Vec3f& position2 = vertices[edge.vIndex1].position;

	// Calculate the squared Euclidean distance as the error metric
	return Vec3f::dist(position1, position2);
}

void Mesh::doEdgeCollapse(std::vector<Vertex>& vertices, std::vector<Edge>& edges, UINT collapseEdgeIndex) {
	// Retrieve the edge to collapse
	Edge& edgeToCollapse = edges[collapseEdgeIndex];

	// Merge the two vertices connected by the edge
	Vertex& vertex1 = vertices[edgeToCollapse.vIndex0];
	Vertex& vertex2 = vertices[edgeToCollapse.vIndex1];

	// Perform vertex position update (e.g., take the midpoint)
	vertex1.position = Vec3f(
		(vertex1.position.x + vertex2.position.x) * 0.5f,
		(vertex1.position.y + vertex2.position.y) * 0.5f,
		(vertex1.position.z + vertex2.position.z) * 0.5f
	);

	// Update the remaining edges to reflect the merged vertex
	for (Edge& edge : edges) {
		if (edge.vIndex0 == edgeToCollapse.vIndex1) {
			edge.vIndex0 = edgeToCollapse.vIndex0;
		}
		
		if (edge.vIndex1 == edgeToCollapse.vIndex1) {
			edge.vIndex1 = edgeToCollapse.vIndex0;
		}
	}

	// Remove the collapsed vertex and the edge from the lists
	vertices.erase(vertices.begin() + edgeToCollapse.vIndex1);
	edges.erase(edges.begin() + collapseEdgeIndex);
	// You may need to update the indices in the remaining edges accordingly
}

void Mesh::simplifyMesh() {
	// Initialize DirectX 11 context, create buffers, load mesh, etc.

	std::vector<Vertex> vertices;
	std::vector<Edge> edges;

	// Populate vertices and edges with your 3D mesh data

	// Iterate through the edges and select the least important ones based on your error metric
	// For simplicity, let's assume you have a function that calculates an error value for each edge
	for (UINT i = 0; i < edges.size(); ++i) {
		Edge& edge = edges[i];
		float edgeError = calculateEdgeError(vertices, edge);  // Implement this function
		// Check if the edgeError is below a threshold or based on other criteria
		float errorThreshold = 0.25f;
		if (edgeError < errorThreshold) {
			doEdgeCollapse(vertices, edges, i);
			// After collapsing an edge, you may need to update the remaining edge error metrics
		}
	}

	// Now, your "vertices" and "edges" vectors contain the simplified mesh
	// You can use this data for rendering or further processing
}

void Mesh::convertMatrix(const aiMatrix4x4& input, Mat4f& output) {
	output.mat[0][0] = input.a1;
	output.mat[0][1] = input.a2;
	output.mat[0][2] = input.a3;
	output.mat[0][3] = input.a4;

	output.mat[1][0] = input.b1;
	output.mat[1][1] = input.b2;
	output.mat[1][2] = input.b3;
	output.mat[1][3] = input.b4;

	output.mat[2][0] = input.c1;
	output.mat[2][1] = input.c2;
	output.mat[2][2] = input.c3;
	output.mat[2][3] = input.c4;

	output.mat[3][0] = input.d1;
	output.mat[3][1] = input.d2;
	output.mat[3][2] = input.d3;
	output.mat[3][3] = input.d4;
}

const SP_VertexBuffer& Mesh::getPositionVertexBuffer() {
	return m_vbPosition;
}

const SP_VertexBuffer& Mesh::getMeshVertexBuffer() {
	return m_vbMesh;
}

const SP_VertexBuffer& Mesh::getBoneVertexBuffer() {
	return m_vbBone;
}

const SP_IndexBuffer& Mesh::getIndexBuffer() {
	return m_ib;
}

const std::vector<SubMesh>& Mesh::getMeshes() {
	return m_meshes;
}

unsigned int Mesh::getTextureIndex() {
	return m_textureIndex;
}

const std::vector<Face>& Mesh::getFaces() {
	return m_faces;
}

std::vector<float>& Mesh::getVertexPositions() {
	return m_positions;
}

std::vector<unsigned int>& Mesh::getVertexIndices() {
	return m_indices;
}
