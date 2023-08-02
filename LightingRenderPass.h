#pragma once

#pragma comment(lib, "d3dcompiler")

#include <d3dcompiler.h>

#include "DXGLMain.h"
#include "RenderPass.h"

namespace dxgl {

	struct alignas(16) MaterialId {
		int materialId = 0;
	};

	class LightingRenderPass : public RenderPass {
	public:
		LightingRenderPass();
		~LightingRenderPass();

		void draw(std::unordered_map<SP_Mesh, std::vector<InstanceTransform>>& instances) override;

	private:
		ID3D11DepthStencilState* m_dsState = nullptr;
		ID3D11RasterizerState* m_rasterState = nullptr;

		SP_InputLayout m_layout = nullptr;
		ID3D11VertexShader* m_vertexShader = nullptr;
		ID3D11PixelShader* m_pixelShader = nullptr;
		SP_VSConstantBuffer m_vcb = nullptr;
		SP_PSConstantBuffer m_pcb = nullptr;

		struct Transform {
			Mat4f view;
			Mat4f proj;
		};
	};
}