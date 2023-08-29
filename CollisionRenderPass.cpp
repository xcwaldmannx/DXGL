#include "CollisionRenderPass.h"

using namespace dxgl;

CollisionRenderPass::CollisionRenderPass() {

	// Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = false;

	HRESULT result = Engine::graphics()->device()->CreateDepthStencilState(&depthStencilDesc, &m_dsState);

	if (FAILED(result)) {
		throw std::runtime_error("Skybox Depth Stencil State could not be created.");
	}

	// Raster State
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	result = Engine::graphics()->device()->CreateRasterizerState(&rasterizerDesc, &m_rasterState);

	if (FAILED(result)) {
		throw std::runtime_error("Lighting Rasterizer State could not be created.");
	}

	// shader byte code
	const char* vertexShaderCode = R"(
		struct VSInput {
			float3 position: POSITION;
			float3 color:    COLOR;
		};

		struct PSInput {
			float4 position: SV_POSITION;
			float3 color:    COLOR;
		};

		cbuffer transform: register(b0) {
			row_major matrix view;
			row_major matrix proj;
		};

		PSInput main(VSInput input) {
			PSInput output = (PSInput)0;

			output.position = mul(float4(input.position, 1.0f), view);
			output.position = mul(output.position, proj);

			output.color = input.color;

			return output;
		}
	)";

	const char* pixelShaderCode = R"(
		struct PSInput {
			float4 position: SV_POSITION;
			float3 color:    COLOR;
		};

		float4 main(PSInput input) : SV_TARGET {
			return float4(input.color, 1.0f);
		}
	)";

	// compile shaders
	ID3DBlob* vsBlob = nullptr;
	result = D3DCompile(vertexShaderCode, strlen(vertexShaderCode), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, nullptr);
	if (FAILED(result)) {
		throw std::runtime_error("Collision Vertex Shader could not be compiled.");
	}

	ID3DBlob* psBlob = nullptr;
	result = D3DCompile(pixelShaderCode, strlen(pixelShaderCode), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, nullptr);
	if (FAILED(result)) {
		throw std::runtime_error("Collision Pixel Shader could not be compiled.");
	}

	// Input layout
	dxgl::InputLayoutDesc descMousePick{};
	descMousePick.add("POSITION", 0, dxgl::FLOAT3, false);
	descMousePick.add("COLOR",    0, dxgl::FLOAT3, false);
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
	m_vcbTransform = Engine::resource()->createVSConstantBuffer(sizeof(Transform));
}

CollisionRenderPass::~CollisionRenderPass() {

}

void CollisionRenderPass::draw(std::vector<Vec3f>& points) {

	// Bind the depth-stencil view and render target view
	SP_DXGLRenderTargetView rtv = Engine::renderer()->getRTV(RESOURCE_VIEW_SLOT_BACK_BUFFER);
	ID3D11RenderTargetView* rtvv = rtv->get();
	SP_DXGLDepthStencilView dsv = Engine::renderer()->getDSV(RESOURCE_VIEW_SLOT_BACK_BUFFER);
	Engine::graphics()->context()->OMSetRenderTargets(1, &rtvv, dsv->get());

	// Step 3: Clear the render target buffer at the beginning of the render pass
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Set your clear color
	//Engine::graphics()->context()->ClearRenderTargetView(rtvv, clearColor);
	//Engine::graphics()->context()->ClearDepthStencilView(dsv->get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Step 4: Render the opaque objects with depth testing and depth writing enabled
	// Bind shaders, vertex buffers, index buffers, etc.
	// Set up the depth-stencil state with depth testing and writing enabled
	// Draw the opaque objects

	// Set the depth/stencil state
	Engine::graphics()->context()->OMSetDepthStencilState(m_dsState, 0);
	Engine::graphics()->context()->RSSetState(m_rasterState);

	// Step 5 (optional): Render the transparent objects without writing to the depth buffer
	// Bind shaders, vertex buffers, index buffers, etc.
	// Set up the depth-stencil state with depth testing enabled, but depth writing disabled
	// Draw the transparent objects

	// bind input layout and shaders
	m_layout->bind();
	Engine::graphics()->context()->VSSetShader(m_vertexShader, nullptr, 0);
	Engine::graphics()->context()->PSSetShader(m_pixelShader, nullptr, 0);

	// set up and bind view proj transform
	SP_Camera cam = Engine::renderer()->camera()->get("primary");
	Transform t{};
	t.view = cam->view();
	t.proj = cam->proj();
	m_vcbTransform->update(&t);
	m_vcbTransform->bind(0);

	// process data if necessary...
	// ...

	std::vector<Vec3f> vbData{};
	int pointCount = 0;
	Vec3f color = { 1, 0, 0 };
	for (auto& point : points) {
		vbData.push_back(point);

		if (pointCount == 0) {
			color = { 1, 0, 0 };
		}

		if (pointCount == 6) {
			color = { 0, 1, 0 };
		}

		if (pointCount == 12) {
			pointCount = 0;
			color = { 1, 0, 0 };
		}

		vbData.push_back(color);
		pointCount++;
	}

	if (vbData.size() > 0) {
		SP_VertexBuffer vb = Engine::resource()->createVertexBuffer(&vbData[0], vbData.size() / 2.0f, sizeof(Vec3f) * 2);
		vb->bind(0);

		Engine::renderer()->drawLineList(points.size(), 0);
	}
}
