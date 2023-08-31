#pragma once

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler")

#include "RenderPass.h"

#include "DXGLDefinitions.h"

namespace dxgl {
	struct Text {
		std::string text{};
		Vec3f color{};
	};

	class HUDRenderPass : public RenderPass<std::vector<Text>> {
	public:
		HUDRenderPass();
		~HUDRenderPass();

		void draw(std::vector<Text>& texts) override;

	private:
		SP_InputLayout m_layout = nullptr;
		ID3D11VertexShader* m_vertexShader = nullptr;
		ID3D11PixelShader* m_pixelShader = nullptr;

		SP_VSConstantBuffer m_vcbTransform = nullptr;

		struct Vertex {
			Vec3f position{};
			Vec2f texcoord{};
		};

	};
}