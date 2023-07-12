#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <any>
#include <typeindex>
#include <fstream>

#include "DXGLDefinitions.h"

#include "DXGLGraphics.h"
#include "DXGLRenderer.h"

#include "DXGLVertexBuffer.h"
#include "DXGLIndexBuffer.h"
#include "DXGLInstanceBuffer.h"
#include "DXGLCBuffer.h"
#include "DXGLInputLayout.h"

#include "DXGLSamplerState.h"
#include "DXGLShaderResourceView.h"

#include "DXGLTexture2D.h"
#include "DXGLTextureCube.h"

#include "DXGLMaterial.h"
#include "DXGLMesh.h"
#include "DXGLBasicMesh.h"

namespace dxgl {

	enum ResourceType {
		VERTEX_BUFFER,
		INDEX_BUFFER,
		INSTANCE_BUFFER,
		CONSTANT_BUFFER,
		INPUT_LAYOUT,

		VERTEX_SHADER,
		HULL_SHADER,
		DOMAIN_SHADER,
		PIXEL_SHADER,

		SAMPLER_STATE,
		RASTER_STATE,
		DEPTH_STENCIL_STATE,
		BLEND_STATE,

		TEXTURE_2D,
		TEXTURE_CUBE,

		MATERIAL,
		MESH,
	};

	class DXGLResourceManager {
	public:
		DXGLResourceManager();
		~DXGLResourceManager();

		template<typename T>
		T get(const std::string& alias);

		template<typename T>
		bool find(const std::string& alias);

		SP_DXGLVertexBuffer createVertexBuffer(void* vertices, int vertexCount, int vertexSize);
		void storeVertexBuffer(void* vertices, int vertexCount, int vertexSize, const std::string& alias);

		SP_DXGLIndexBuffer createIndexBuffer(void* indices, int indexCount);
		void storeIndexBuffer(void* indices, int indexCount, const std::string& alias);

		SP_DXGLInstanceBuffer createInstanceBuffer(void* instances, int instanceCount, int instanceSize);
		void storeInstanceBuffer(void* instances, int instanceCount, int instanceSize, const std::string& alias);

		SP_DXGLCBuffer createCBuffer(UINT bytes);
		void storeCBuffer(UINT bytes, std::string alias);

		SP_DXGLInputLayout createInputLayout(const InputLayoutDesc& desc, const std::string& filename);
		void storeInputLayout(const InputLayoutDesc& desc, const std::string& filename, const std::string& alias);

		template<class T>
		std::shared_ptr<T> createShader(const std::string& filename);

		template<class T>
		void storeShader(const std::string& filename, const std::string& alias);

		SP_DXGLSamplerState createSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressUVW,
			D3D11_COMPARISON_FUNC comparison, FLOAT borderColor);
		void storeSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressUVW,
			D3D11_COMPARISON_FUNC comparison, FLOAT borderColor, const std::string& alias);

		void createRasterState();
		void storeRasterState();

		void createDepthStencilState();
		void storeDepthStencilState();

		void createBlendState();
		void storeBlendState();

		SP_DXGLTexture2D createTexture2D(const std::string& filename);
		SP_DXGLTexture2D createTexture2D(unsigned int width, unsigned int height, unsigned char* data);
		void storeTexture2D(const std::string& filename, const std::string& alias);
		void storeTexture2D(unsigned int width, unsigned int height, unsigned char* data, const std::string& alias);

		SP_DXGLTextureCube createTextureCube(const std::string& filename);
		void storeTextureCube(const std::string& filename, const std::string& alias);

		SP_DXGLMaterial createMaterial(const std::string& filepath);
		SP_DXGLMaterial createMaterial(const MaterialData& data);
		void storeMaterial(const std::string& filepath, const std::string& alias);
		void storeMaterial(const MaterialData& data, const std::string& alias);

		SP_DXGLMesh createMesh(const std::string& filepath);
		SP_DXGLBasicMesh createBasicMesh(const std::string& filepath);
		void storeMesh(const std::string& filepath, const std::string& alias);
		void storeBasicMesh(const std::string& filepath, const std::string& alias);

	private:
		std::unordered_map<std::type_index, std::unordered_map<std::string, std::any>> m_resources{};

	};
}