#include "MousePickRenderPass.h"

using namespace dxgl;

MousePickRenderPass::MousePickRenderPass() {

	// Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = false;

	HRESULT result = DXGLMain::graphics()->device()->CreateDepthStencilState(&depthStencilDesc, &m_dsState);

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

	result = DXGLMain::graphics()->device()->CreateRasterizerState(&rasterizerDesc, &m_rasterState);

	if (FAILED(result)) {
		throw std::runtime_error("Lighting Rasterizer State could not be created.");
	}

	// shader byte code
	const char* vertexShaderCode = R"(
		struct VSInput {
			float3 position: POSITION;
			uint1  iID: INSTANCE_ID;
			float3 iS:  INSTANCE_S;
			float3 iR:  INSTANCE_R;
			float3 iT:  INSTANCE_T;
		};

		struct PSInput {
			float4 position: SV_POSITION;
			float3 colorId:  COLOR_ID;
		};

		cbuffer transform: register(b0) {
			row_major matrix view;
			row_major matrix proj;
		};

		matrix getWorldMatrix(float3 scale, float3 rotation, float3 translation) {
			matrix scaleMatrix = matrix(
				float4(scale.x, 0.0, 0.0, 0.0),
				float4(0.0, scale.y, 0.0, 0.0),
				float4(0.0, 0.0, scale.z, 0.0),
				float4(0.0, 0.0, 0.0, 1.0)
				);

			matrix rotationMatrix = matrix(
				float4(cos(rotation.y) * cos(rotation.z), cos(rotation.y) * sin(rotation.z), -sin(rotation.y), 0.0),
				float4(sin(rotation.x) * sin(rotation.y) * cos(rotation.z) - cos(rotation.x) * sin(rotation.z),
					sin(rotation.x) * sin(rotation.y) * sin(rotation.z) + cos(rotation.x) * cos(rotation.z),
					sin(rotation.x) * cos(rotation.y), 0.0),
				float4(cos(rotation.x) * sin(rotation.y) * cos(rotation.z) + sin(rotation.x) * sin(rotation.z),
					cos(rotation.x) * sin(rotation.y) * sin(rotation.z) - sin(rotation.x) * cos(rotation.z),
					cos(rotation.x) * cos(rotation.y), 0.0),
				float4(0.0, 0.0, 0.0, 1.0)
				);

			matrix translationMatrix = matrix(
				float4(1.0, 0.0, 0.0, 0.0),
				float4(0.0, 1.0, 0.0, 0.0),
				float4(0.0, 0.0, 1.0, 0.0),
				float4(translation.x, translation.y, translation.z, 1.0)
				);

			return mul(mul(scaleMatrix, rotationMatrix), translationMatrix);
		}

		PSInput main(VSInput input) {
			PSInput output = (PSInput)0;

			matrix worldMatrix = getWorldMatrix(input.iS, input.iR, input.iT);

			output.position = mul(float4(input.position, 1.0f), worldMatrix);
			output.position = mul(output.position, view);
			output.position = mul(output.position, proj);

			float3 instanceColorId = float3(0, 0, 0);
			instanceColorId.r = ((input.iID >> 16) & 0xFF) / 255.0f;
			instanceColorId.g = ((input.iID >>  8) & 0xFF) / 255.0f;
			instanceColorId.b = ((input.iID >>  0) & 0xFF) / 255.0f;

			output.colorId = instanceColorId;

			return output;
		}
	)";

	const char* pixelShaderCode = R"(
		struct PSInput {
			float4 position: SV_POSITION;
			float3 colorId:  COLOR_ID;
		};

		float4 main(PSInput input) : SV_TARGET {
			return float4(input.colorId, 1.0f);
		}
	)";

	// compile shaders
	ID3DBlob* vsBlob = nullptr;
	result = D3DCompile(vertexShaderCode, strlen(vertexShaderCode), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, nullptr);
	if (FAILED(result)) {
		throw std::runtime_error("Mouse Pick Vertex Shader could not be compiled.");
	}

	ID3DBlob* psBlob = nullptr;
	result = D3DCompile(pixelShaderCode, strlen(pixelShaderCode), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, nullptr);
	if (FAILED(result)) {
		throw std::runtime_error("Mouse Pick Pixel Shader could not be compiled.");
	}

	// Input layout
	dxgl::InputLayoutDesc descMousePick{};
	descMousePick.add("POSITION",    0, dxgl::FLOAT3, false);
	descMousePick.add("INSTANCE_ID", 1, dxgl::UINT1,  true);
	descMousePick.add("INSTANCE_S",  1, dxgl::FLOAT3, true);
	descMousePick.add("INSTANCE_R",  1, dxgl::FLOAT3, true);
	descMousePick.add("INSTANCE_T",  1, dxgl::FLOAT3, true);
	m_layout = dxgl::DXGLMain::resource()->createInputLayout(descMousePick, vsBlob);

	// create shaders
	result = DXGLMain::graphics()->device()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(result)) {
		throw std::runtime_error("Mouse Pick Vertex Shader could not be created.");
	}
	vsBlob->Release();

	result = DXGLMain::graphics()->device()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(result)) {
		throw std::runtime_error("Mouse Pick Pixel Shader could not be created.");
	}
	psBlob->Release();

	// transform buffer
	m_vcbTransform = DXGLMain::resource()->createVSConstantBuffer(sizeof(Transform));
}

MousePickRenderPass::~MousePickRenderPass() {

}

void MousePickRenderPass::draw(std::unordered_map<SP_Mesh, std::vector<PerInstanceData>>& instances) {

	if (!m_RTV && !m_DSV) {
		// views
		RESOURCE_VIEW_DESC rtvmpDesc = {
			UNORM8,
			1,
			0,
		};
		DXGLMain::renderer()->createRenderTargetView(&rtvmpDesc, RESOURCE_VIEW_SLOT_1, &m_RTV);

		RESOURCE_VIEW_DESC dsvmpDesc = {
			D24_S8,
			1,
			0,
		};
		DXGLMain::renderer()->createDepthStencilView(&dsvmpDesc, RESOURCE_VIEW_SLOT_1, &m_DSV);
	}

	// Bind the depth-stencil view and render target view
	SP_DXGLRenderTargetView rtv = DXGLMain::renderer()->getRTV(RESOURCE_VIEW_SLOT_1);
	ID3D11RenderTargetView* rtvv = rtv->get();
	SP_DXGLDepthStencilView dsv = DXGLMain::renderer()->getDSV(RESOURCE_VIEW_SLOT_1);
	DXGLMain::graphics()->context()->OMSetRenderTargets(1, &rtvv, dsv->get());

	// Step 3: Clear the render target buffer at the beginning of the render pass
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Set your clear color
	DXGLMain::graphics()->context()->ClearRenderTargetView(rtvv, clearColor);
	DXGLMain::graphics()->context()->ClearDepthStencilView(dsv->get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Step 4: Render the opaque objects with depth testing and depth writing enabled
	// Bind shaders, vertex buffers, index buffers, etc.
	// Set up the depth-stencil state with depth testing and writing enabled
	// Draw the opaque objects

	// Set the depth/stencil state
	DXGLMain::graphics()->context()->OMSetDepthStencilState(m_dsState, 0);
	DXGLMain::graphics()->context()->RSSetState(m_rasterState);

	// Step 5 (optional): Render the transparent objects without writing to the depth buffer
	// Bind shaders, vertex buffers, index buffers, etc.
	// Set up the depth-stencil state with depth testing enabled, but depth writing disabled
	// Draw the transparent objects

	// bind input layout and shaders
	m_layout->bind();
	DXGLMain::graphics()->context()->VSSetShader(m_vertexShader, nullptr, 0);
	DXGLMain::graphics()->context()->PSSetShader(m_pixelShader, nullptr, 0);

	// set up and bind view proj transform
	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");
	Transform t{};
	t.view = cam->view();
	t.proj = cam->proj();
	m_vcbTransform->update(&t);
	m_vcbTransform->bind(0);

	// process entities if necessary...
	// ...

		// combine all instances in one list
	std::vector<PerInstanceData> combinedInstances{};
	for (const auto& pair : instances) {
		const std::vector<PerInstanceData>& list = pair.second;
		combinedInstances.insert(combinedInstances.end(), list.begin(), list.end());
	}

	// create and bind instance buffer
	SP_InstanceBuffer instanceBuffer = DXGLMain::resource()->createInstanceBuffer(&combinedInstances[0], combinedInstances.size(), sizeof(PerInstanceData));
	instanceBuffer->bind(1);

	int entityCount = 0;
	for (const auto& instance : instances) {
		const SP_Mesh& mesh = instance.first;
		// bind entity group data
		mesh->getPositionVertexBuffer()->bind(0);
		mesh->getIndexBuffer()->bind();

		// get sub meshes
		for (auto& subMesh : mesh->getMeshes()) {
			// draw sub meshes
			DXGLMain::renderer()->drawIndexedTriangleListInstanced(subMesh.indexCount, instances[mesh].size(), subMesh.baseIndex, subMesh.baseVertex, entityCount);
		}
		entityCount += instances[mesh].size();
	}
}
