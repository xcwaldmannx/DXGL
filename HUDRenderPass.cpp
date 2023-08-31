#include "HUDRenderPass.h"

#include "Engine.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "TextRenderManager.h"

using namespace dxgl;

HUDRenderPass::HUDRenderPass() {
	// shader code
	const char* vertexShaderCode = R"(
		struct VS_Input {
			float3 position: POSITION;
			float2 texcoord: TEXCOORD;
		};

		struct PS_Input {
			float4 position: SV_POSITION;
			float2 texcoord: TEXCOORD;
		};

		PS_Input main(VS_Input input) {
			PS_Input output = (PS_Input)0;

			output.position = float4(input.position, 1.0f);
			output.texcoord = input.texcoord;

			return output;
		}
	)";

	const char* pixelShaderCode = R"(
		SamplerState textureSampler: register(s0);
		Texture2D    tex:            register(t0);

		struct PS_Input {
			float4 position: SV_POSITION;
			float2 texcoord: TEXCOORD;
		};

		float4 main(PS_Input input) : SV_TARGET {

			float4 color = tex.Sample(textureSampler, input.texcoord);

			if (color.a == 0) discard;

			return float4(color.rgb, 1.0f);
		}
	)";

	// compile shaders
	ID3DBlob* vsBlob = nullptr;
	HRESULT result = D3DCompile(vertexShaderCode, strlen(vertexShaderCode), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, nullptr);
	if (FAILED(result)) {
		throw std::runtime_error("Lighting Vertex Shader could not be compiled.");
	}

	ID3DBlob* psBlob = nullptr;
	result = D3DCompile(pixelShaderCode, strlen(pixelShaderCode), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, nullptr);
	if (FAILED(result)) {
		throw std::runtime_error("Lighting Pixel Shader could not be compiled.");
	}

	// Input layout
	dxgl::InputLayoutDesc descMousePick{};
	descMousePick.add("POSITION", 0, dxgl::FLOAT3, false);
	descMousePick.add("TEXCOORD", 0, dxgl::FLOAT2, false);
	m_layout = dxgl::Engine::resource()->createInputLayout(descMousePick, vsBlob);

	// create shaders
	result = Engine::graphics()->device()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(result)) {
		throw std::runtime_error("Collision Vertex Shader could not be created.");
	}
	vsBlob->Release();

	result = Engine::graphics()->device()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(result)) {
		throw std::runtime_error("Collision Pixel Shader could not be created.");
	}
	psBlob->Release();

	// transform buffer
	// m_vcbTransform = Engine::resource()->createVSConstantBuffer(sizeof(Transform));
}

HUDRenderPass::~HUDRenderPass() {

}

void HUDRenderPass::draw(std::vector<Text>& texts) {
	// Step 2: Bind the depth-stencil view and render target view to the output merger stage
	SP_DXGLRenderTargetView rtv = Engine::renderer()->getRTV(RESOURCE_VIEW_SLOT_BACK_BUFFER);
	ID3D11RenderTargetView* rtvv = rtv->get();
	SP_DXGLDepthStencilView dsv = Engine::renderer()->getDSV(RESOURCE_VIEW_SLOT_BACK_BUFFER);
	Engine::graphics()->context()->OMSetRenderTargets(1, &rtvv, dsv->get());

	// Set the depth/stencil state
	// Engine::graphics()->context()->OMSetDepthStencilState(m_dsState, 0);
	// Engine::graphics()->context()->RSSetState(m_rasterState);

	// bind input layout and shaders
	m_layout->bind();
	Engine::graphics()->context()->VSSetShader(m_vertexShader, nullptr, 0);
	Engine::graphics()->context()->PSSetShader(m_pixelShader, nullptr, 0);

	std::vector<Vertex> vertices = {
	{ {-1.0f, 1.0f, 0.0f }, {0, 0} },
	{ { 1.0f, 1.0f, 0.0f }, {1, 0} },
	{ { 1.0f,-1.0f, 0.0f }, {1, 1} },
	{ {-1.0f,-1.0f, 0.0f }, {0, 1} },
	};

	SP_VertexBuffer vertexBuffer = dxgl::Engine::resource()->createVertexBuffer(&vertices[0], vertices.size(), sizeof(Vertex));

	std::vector<unsigned int> indices = {
		0, 1, 2, 2, 3, 0
	};

	SP_IndexBuffer indexBuffer = dxgl::Engine::resource()->createIndexBuffer(&indices[0], indices.size());

	vertexBuffer->bind(0);
	indexBuffer->bind();

	Engine::textrender()->renderText(L"", {0, 0, 0});

	for (auto& text : texts) {
		std::wstring widestr = std::wstring(text.text.begin(), text.text.end());
		const wchar_t* widecstr = widestr.c_str();
		Engine::textrender()->renderText(widecstr, text.color);
	}

	Engine::renderer()->shader()->PS_setResource(0, Engine::textrender()->getSRV());

	Engine::renderer()->drawIndexedTriangleList(6, 0, 0);
}
