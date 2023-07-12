#pragma once

#include <string>
#include <unordered_map>

#include "DXGLGraphics.h"
#include "DXGLDefinitions.h"

#include "assimp/material.h"

namespace dxgl {

	enum TextureType {
		NONE        = aiTextureType_UNKNOWN,
		NORMAL_DISP = aiTextureType_NORMALS,
		ALBEDO      = aiTextureType_DIFFUSE,
		METALLIC    = aiTextureType_METALNESS,
		ROUGHNESS   = aiTextureType_SHININESS,
		AMB_OCC     = aiTextureType_SPECULAR,
	};

	enum MaterialFlag {
		MATERIAL_FLAG_ZERO          =  0,
		MATERIAL_FLAG_USE_NORM_DISP =  1,
		MATERIAL_FLAG_USE_ALBEDO    =  2,
		MATERIAL_FLAG_USE_METALLIC  =  4,
		MATERIAL_FLAG_USE_ROUGHNESS =  8,
		MATERIAL_FLAG_USE_AMB_OCC   = 16,
	};

	struct TextureData {
		TextureType type = NONE;
		int width = 0;
		int height = 0;
		int channels = 0;
		unsigned char* data = nullptr;
	};

	struct MaterialData {
		MaterialFlag id = MATERIAL_FLAG_ZERO;
		std::vector<TextureData> textures;
	};

	class DXGLMaterial {
	public:
		DXGLMaterial(const std::string& filepath, SP_DXGLGraphics graphics);
		DXGLMaterial(const MaterialData& data, SP_DXGLGraphics graphics);
		~DXGLMaterial();

		ID3D11ShaderResourceView* get();

		unsigned int materialIndices() {
			return m_materialIndices;
		}

	private:
		ID3D11ShaderResourceView* m_srv = nullptr;
		unsigned int m_materialIndices = 0;
	};
}
