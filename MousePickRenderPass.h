#pragma once

#pragma comment(lib, "d3dcompiler")

#include <d3dcompiler.h>

#include "RenderPass.h"

namespace dxgl {

	class MousePickRenderPass : public RenderPass<std::unordered_map<SP_Mesh, std::vector<PerInstanceData>>> {
	public:
		MousePickRenderPass();
		~MousePickRenderPass();

		void draw(std::unordered_map<SP_Mesh, std::vector<PerInstanceData>>& instances) override;

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

		struct InstanceMousePick {
			unsigned int id = 0;
			Vec3f scale{};
			Vec3f rotation{};
			Vec3f translation{};
		};
	};
}