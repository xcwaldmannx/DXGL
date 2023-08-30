#include "EarlyZRenderPass.h"

#include "Engine.h"
#include "Graphics.h"
#include "ResourceManager.h"
#include "CameraManager.h"

using namespace dxgl;

EarlyZRenderPass::EarlyZRenderPass() {
	// create depth stencil state
	// Define the depth/stencil state description
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // Use D3D11_COMPARISON_LESS_EQUAL

	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	// Configure stencil operations (if needed, set StencilEnable to true)
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HRESULT result = Engine::graphics()->device()->CreateDepthStencilState(&depthStencilDesc, &m_dsState);

	if (FAILED(result)) {
		throw std::runtime_error("Early-Z Depth Stencil State could not be created.");
	}

	// Create the rasterizer state
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
		throw std::runtime_error("Early-Z Rasterizer State could not be created.");
	}

	// shader code
	const char* vertexShaderCode = R"(
    cbuffer transform : register(b0)  {
        row_major matrix view;
        row_major matrix proj;
    }

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

    struct VSInput {
        float3 position : POSITION;
		uint1  iID : INSTANCE_ID;
		float3 iS  : INSTANCE_S;
		float3 iR  : INSTANCE_R;
		float3 iT  : INSTANCE_T;
    };

    struct PSInput {
        float4 position : SV_POSITION;
		float1 depth : DEPTH;
    };

    PSInput main(VSInput input) {
        PSInput output = (PSInput)0;
		matrix world = getWorldMatrix(input.iS, input.iR, input.iT);
        output.position = mul(float4(input.position, 1.0f), world);
        output.position = mul(output.position, view);
        output.position = mul(output.position, proj);

		output.depth = (output.position.z / output.position.w + 1.0f) * 0.5f;

        return output;
    }
	)";

	// compile shaders
	ID3DBlob* vsBlob = nullptr;
	result = D3DCompile(vertexShaderCode, strlen(vertexShaderCode), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, nullptr);
	if (FAILED(result)) {
		throw std::runtime_error("Early Z Vertex Shader could not be compiled.");
	}

	// create input layout
	InputLayoutDesc ilDesc{};
	ilDesc.add("POSITION", 0, FLOAT3, false);
	ilDesc.add("INSTANCE_ID", 1, UINT1,  true);
	ilDesc.add("INSTANCE_S",  1, FLOAT3, true);
	ilDesc.add("INSTANCE_R",  1, FLOAT3, true);
	ilDesc.add("INSTANCE_T",  1, FLOAT3, true);

	m_layout = Engine::resource()->createInputLayout(ilDesc, vsBlob);

	// create shaders
	result = Engine::graphics()->device()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(result)) {
		throw std::runtime_error("Early Z Vertex Shader could not be created.");
	}
	vsBlob->Release();

	// create constant buffers
	m_vcb = Engine::resource()->createVSConstantBuffer(sizeof(Transform));
}

EarlyZRenderPass::~EarlyZRenderPass() {
	m_dsState->Release();
	m_rasterState->Release();
	m_vertexShader->Release();
}

void EarlyZRenderPass::draw(std::unordered_map<SP_Mesh, std::vector<PerInstanceData>>& instances) {
	// Step 2: Bind the depth-stencil view to the output merger stage
	SP_DXGLDepthStencilView dsv = Engine::renderer()->getDSV(RESOURCE_VIEW_SLOT_BACK_BUFFER);
	Engine::graphics()->context()->OMSetRenderTargets(0, nullptr, dsv->get());

	// Step 3: Clear the depth buffer at the beginning of the render pass
	// Engine::graphics()->context()->ClearDepthStencilView(dsv->get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

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

	// set up and bind view proj transform
	auto& cam = Engine::camera()->getActiveCamera();
	Transform t{};
	t.view = cam.view();
	t.proj = cam.proj();
	m_vcb->update(&t);
	m_vcb->bind(0);

	// sort entities if necessary...
	// ...

	// combine all instances in one list
	std::vector<PerInstanceData> combinedInstances{};
	for (const auto& pair : instances) {
		const std::vector<PerInstanceData>& list = pair.second;
		combinedInstances.insert(combinedInstances.end(), list.begin(), list.end());
	}

	// create and bind instance buffer
	SP_InstanceBuffer instanceBuffer = Engine::resource()->createInstanceBuffer(&combinedInstances[0], combinedInstances.size(), sizeof(PerInstanceData));
	instanceBuffer->bind(1);

	int triangleCount = 0;
	int entityCount = 0;
	for (const auto& instance : instances) {
		const SP_Mesh& mesh = instance.first;
		// bind entity group data
		mesh->getMeshVertexBuffer()->bind(0);
		mesh->getIndexBuffer()->bind();

		// draw entities
		for (auto& subMesh : mesh->getMeshes()) {
			Engine::renderer()->drawIndexedTriangleListInstanced(subMesh.indexCount, instances[mesh].size(), subMesh.baseIndex, subMesh.baseVertex, entityCount);
		}

		triangleCount += (mesh->getIndexBuffer()->count() / 3) * instances[mesh].size();
		entityCount += instances[mesh].size();
	}

	// reset/clean up
	Engine::graphics()->context()->OMSetRenderTargets(0, nullptr, nullptr);

	Engine::graphics()->context()->OMSetDepthStencilState(nullptr, 0);
	Engine::graphics()->context()->RSSetState(nullptr);

	Engine::graphics()->context()->VSSetShader(nullptr, nullptr, 0);
	Engine::graphics()->context()->PSSetShader(nullptr, nullptr, 0);
}
