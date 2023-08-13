#pragma once

#include "RenderPass.h"

namespace dxgl {
	class SkyboxRenderPass : public RenderPass {
	public:
		SkyboxRenderPass();
		~SkyboxRenderPass();

		void draw(std::unordered_map<SP_Mesh, std::vector<PerInstanceData>>& instances) override;

	private:
		ID3D11DepthStencilState* m_dsState = nullptr;
		ID3D11RasterizerState* m_rasterState = nullptr;

		SP_InputLayout m_layout = nullptr;
		SP_DXGLVertexShader m_vertexShader = nullptr;
		SP_DXGLPixelShader m_pixelShader = nullptr;
		SP_VSConstantBuffer m_vcbTransform = nullptr;

		SP_TextureCube m_skybox = nullptr;

		struct Transform {
			Mat4f world;
			Mat4f view;
			Mat4f proj;
		};
	};
}