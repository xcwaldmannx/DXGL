#pragma once

#include <string>
#include <unordered_map>

#include "Engine.h"
#include "Bindable.h"

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

	class Material : public Bindable {
	public:
		Material(const std::string& filepath);
		Material(const MaterialData& data);
		~Material();

		void bind(int slot) override;

		ID3D11ShaderResourceView* get();

		unsigned int materialIndices() {
			return m_materialIndices;
		}

	private:
		ID3D11ShaderResourceView* m_srv = nullptr;
		unsigned int m_materialIndices = 0;
	};
}
