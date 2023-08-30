#pragma once

#pragma comment(lib, "d3dcompiler")

#include <d3dcompiler.h>

#include "RenderPass.h"

namespace dxgl {

	struct alignas(16) MaterialId {
		unsigned int textureIndex = 0;
		float amountMetallic      = 0;
		float amountRoughness     = 1;
		float pad0;
		Vec3f camPosition{};
		float pad1;
	};

	class LightingRenderPass : public RenderPass<std::unordered_map<SP_Mesh, std::vector<PerInstanceData>>> {
	public:
		LightingRenderPass();
		~LightingRenderPass();

		void draw(std::unordered_map<SP_Mesh, std::vector<PerInstanceData>>& instances) override;

	private:
		ID3D11DepthStencilState* m_dsState = nullptr;
		ID3D11RasterizerState* m_rasterState = nullptr;

		SP_InputLayout m_layout = nullptr;
		SP_DXGLVertexShader m_vertexShader = nullptr;
		SP_DXGLPixelShader m_pixelShader = nullptr;
		SP_VSConstantBuffer m_vcbTransform = nullptr;
		SP_PSConstantBuffer m_pcbMaterial = nullptr;

		SP_Texture2D m_brdf = nullptr;

		struct Transform {
			Mat4f view;
			Mat4f proj;
		};
	};
}