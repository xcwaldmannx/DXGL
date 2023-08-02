#pragma once

#include <map>
#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "DXGLMain.h"
#include "DXGLImage.h"
#include "Material.h"

#include "Vertex.h"
#include "Vec2f.h"
#include "Vec3f.h"
#include "Mat4f.h"

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace)

#define MAX_BONES_PER_VERTEX 4
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

namespace dxgl {

	enum VertexAttribute {
		VERTEX_ZERO     =  0,
		VERTEX_POSITION =  1,
		VERTEX_TEXCOORD =  2,
		VERTEX_NORMAL   =  4,
		VERTEX_TANGENT  =  8,
		VERTEX_ALL = VERTEX_POSITION | VERTEX_TEXCOORD | VERTEX_NORMAL | VERTEX_TANGENT,
	};

	enum MiscAttribute {
		MISC_ZERO      = 0,
		MISC_INDEX     = 1,
		MISC_MATERIAL  = 2,
		MISC_ANIMATION = 4,
		MISC_FACE      = 8,
		MISC_DEFAULT = MISC_INDEX | MISC_MATERIAL | MISC_ANIMATION,
		MISC_ALL     = MISC_INDEX | MISC_MATERIAL | MISC_ANIMATION | MISC_FACE,
	};

	struct MeshDesc {
		unsigned int vertexAttributes = 0;
		unsigned int miscAttributes   = 0;
	};

	struct BasicMesh {
		unsigned int indexCount    = 0;
		unsigned int baseVertex    = 0;
		unsigned int baseIndex     = 0;
		unsigned int materialIndex = 0;
		std::string materialName;
	};

	struct AABB {
		Vec3f min{};
		Vec3f max{};
	};

	struct Face {
		Vec3f indices;
		Vec3f v0;
		Vec3f v1;
		Vec3f v2;
	};

	struct VertexBoneData {
		unsigned int boneIds[MAX_BONES_PER_VERTEX];
		float weights[MAX_BONES_PER_VERTEX];

		void addBoneData(unsigned int boneId, float weight) {
			for (unsigned int i = 0; i < COUNT_OF(boneIds); i++) {
				if (weights[i] == 0) {
					boneIds[i] = boneId;
					weights[i] = weight;
					return;
				}
			}
		}
	};

	struct MatrixBoneData {
		Mat4f offset;
		Mat4f transform;
	};

	class Mesh {
	public:
		Mesh(const MeshDesc& desc, const std::string& filename);
		~Mesh();

		const SP_VertexBuffer& getMeshVertexBuffer();
		const SP_VertexBuffer& getBoneVertexBuffer();
		const SP_IndexBuffer& getIndexBuffer();

		const std::vector<BasicMesh>& getMeshes();
		void getBoneTransforms(unsigned int animationIndex, long double deltaTime, std::vector<Mat4f>& transforms);
		int getUsedMaterials();
		const std::vector<Face>& getFaces();

		const AABB getAABB();

	private:
		void loadMaterialTextures(const aiScene* scene);

		void loadBones(unsigned int meshIndex, const aiMesh* mesh);
		void readNodeHierarchy(unsigned int animationIndex, long double animationTimeTicks, const aiNode* node, Mat4f& parentTransform);

		const aiNodeAnim* findNodeAnim(const aiAnimation* animation, const std::string& nodeName);
		void calcInterpolatedScale(aiVector3D& scale, long double animationTimeTicks, const aiNodeAnim* nodeAnim);
		void calcInterpolatedRotation(aiQuaternion& rotation, long double animationTimeTicks, const aiNodeAnim* nodeAnim);
		void calcInterpolatedTranslation(aiVector3D& translation, long double animationTimeTicks, const aiNodeAnim* nodeAnim);

		void computeAxialMinAndMax(const Vec3f& vec);
		void computeAABB();

		void convertMatrix(const aiMatrix4x4& input, Mat4f& output);

	private:
		Assimp::Importer m_importer;
		const aiScene* m_scene;

		SP_VertexBuffer m_vbMesh = nullptr; // slot 0
		SP_VertexBuffer m_vbBone = nullptr; // slot 1
		SP_VertexBuffer m_vbMtl = nullptr;  // slot 2
		SP_IndexBuffer m_ib = nullptr;

		std::vector<BasicMesh> m_meshes{};
		std::vector<std::string> m_materialNames{};

		std::vector<float> m_vertices{};
		std::vector<unsigned int> m_indices{};
		std::vector<Face> m_faces{};

		int m_usedMaterials = 0;

		std::vector<VertexBoneData> m_bones{};
		std::vector<MatrixBoneData> m_boneTransforms{};
		std::vector<int> m_meshBaseVertex{};
		std::unordered_map<std::string, unsigned int> m_boneNameToIndex{};

		Mat4f m_globalInverseTransform{};

		Vec3f m_axialMin{};
		Vec3f m_axialMax{};
	};
}