#pragma once

#pragma comment(lib, "d3dcompiler")

#include <d3dcompiler.h>

#include "DXGLMain.h"
#include "RenderPass.h"

namespace dxgl {

	class CollisionRenderPass : public RenderPass<std::vector<Vec3f>> {
	public:
		CollisionRenderPass();
		~CollisionRenderPass();

		void draw(std::vector<Vec3f>& points) override;

	private:
		governor::DXGLGroup* m_entities{};

		ID3D11DepthStencilState* m_dsState = nullptr;
		ID3D11RasterizerState* m_rasterState = nullptr;

		SP_InputLayout m_layout = nullptr;
		ID3D11VertexShader* m_vertexShader = nullptr;
		ID3D11PixelShader* m_pixelShader = nullptr;
		SP_VSConstantBuffer m_vcbTransform = nullptr;

		SP_DXGLRenderTargetView m_RTV = nullptr;
		SP_DXGLDepthStencilView m_DSV = nullptr;

		struct Transform {
			Mat4f view;
			Mat4f proj;
		};
	};
}