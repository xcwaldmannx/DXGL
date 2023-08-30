#include"LightingRenderPass.h"

#include "Engine.h"
#include "Graphics.h"
#include "ResourceManager.h"
#include "CameraManager.h"

using namespace dxgl;

LightingRenderPass::LightingRenderPass() {
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};

	// Configure the depth test settings
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // Disable depth writes
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	// Optional: Configure stencil test settings if needed
	depthStencilDesc.StencilEnable = false; // Set to TRUE if you need stencil testing
	// ... Configure stencil settings here if enabled ...

	// Create the depth-stencil state object
	HRESULT result = Engine::graphics()->device()->CreateDepthStencilState(&depthStencilDesc, &m_dsState);

	if (FAILED(result)) {
		throw std::runtime_error("Lighting Depth Stencil State could not be created.");
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
		throw std::runtime_error("Lighting Rasterizer State could not be created.");
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
        float2 texcoord : TEXCOORD;
		uint1  iID : INSTANCE_ID;
		float3 iS  : INSTANCE_S;
		float3 iR  : INSTANCE_R;
		float3 iT  : INSTANCE_T;

		uint vertexId : SV_VERTEXID;
    };

    struct PSInput {
        float4 position : SV_POSITION;
		float2 texcoord : TEXCOORD;
		nointerpolation float4 color : COLOR;
    };

    PSInput main(VSInput input) {
        PSInput output = (PSInput)0;
		matrix world = getWorldMatrix(input.iS, input.iR, input.iT);
        output.position = mul(float4(input.position, 1.0f), world);
        output.position = mul(output.position, view);
        output.position = mul(output.position, proj);

		output.texcoord = input.texcoord;

		// Assign a unique color for each triangle based on the SV_VertexID
		// SV_VertexID will contain the index of the vertex within the primitive (0, 1, or 2 for triangles)
		float triangleID = floor(input.vertexId / 3); // Divide by 3 to get the triangle ID
		output.color = float4(triangleID % 2, (triangleID / 2) % 2, (triangleID / 4) % 2, 1); // Unique RGB color per triangle

        return output;
    }
	)";

	const char* pixelShaderCode = R"(
	SamplerState textureSampler;
	Texture2DArray<float4> material : register(t0);

    struct PSInput {
        float4 position : SV_POSITION;
		float2 texcoord : TEXCOORD;
		nointerpolation float4 color : COLOR;
    };

    float4 main(PSInput input) : SV_TARGET {
		float3 color = material.Sample(textureSampler, float3(input.texcoord, 1)).rgb;

		// HDR tone mapping and gamma correction
		float exposure = 2.0f;
		float gamma = 2.2333f;

		float3 toneMap = float3(1.0f, 1.0f, 1.0f) - exp((-color) * exposure);
		float3 toneMappedColor = pow(toneMap, gamma);

        return float4(toneMappedColor, 1.0f);
    }
	)";

	/*

	// compile shaders
	ID3DBlob* vsBlob = nullptr;
	result = D3DCompile(vertexShaderCode, strlen(vertexShaderCode), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, nullptr);
	if (FAILED(result)) {
		throw std::runtime_error("Lighting Vertex Shader could not be compiled.");
	}

	ID3DBlob* psBlob = nullptr;
	result = D3DCompile(pixelShaderCode, strlen(pixelShaderCode), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, nullptr);
	if (FAILED(result)) {
		throw std::runtime_error("Lighting Pixel Shader could not be compiled.");
	}
	*/

	m_vertexShader = Engine::resource()->createShader<DXGLVertexShader>("Assets/Shaders/VSMaterialShader.cso");
	m_pixelShader = Engine::resource()->createShader<DXGLPixelShader>("Assets/Shaders/PSMaterialShader.cso");

	// create input layout
	InputLayoutDesc ilDesc{};
	ilDesc.add("POSITION", 0, FLOAT3, false);
	ilDesc.add("TEXCOORD", 0, FLOAT2, false);
	ilDesc.add("NORMAL",   0, FLOAT3, false);
	ilDesc.add("TANGENT",  0, FLOAT3, false);
	ilDesc.add("INSTANCE_ID", 1, UINT1,  true);
	ilDesc.add("INSTANCE_S",  1, FLOAT3, true);
	ilDesc.add("INSTANCE_R",  1, FLOAT3, true);
	ilDesc.add("INSTANCE_T",  1, FLOAT3, true);

	m_layout = Engine::resource()->createInputLayout(ilDesc, "Assets/Shaders/VSMaterialShader.cso");

	/*

	// create shaders
	result = Engine::graphics()->device()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(result)) {
		throw std::runtime_error("Early Z Vertex Shader could not be created.");
	}
	vsBlob->Release();

	result = Engine::graphics()->device()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(result)) {
		throw std::runtime_error("Early Z Pixel Shader could not be created.");
	}
	psBlob->Release();

	*/

	// create constant buffers
	m_vcbTransform = Engine::resource()->createVSConstantBuffer(sizeof(Transform));
	m_pcbMaterial = Engine::resource()->createPSConstantBuffer(sizeof(MaterialId));

	m_brdf = Engine::resource()->createTexture2D("Assets/Textures/brdf.png");
}

LightingRenderPass::~LightingRenderPass() {
	m_dsState->Release();
	m_rasterState->Release();
}

void LightingRenderPass::draw(std::unordered_map<SP_Mesh, std::vector<PerInstanceData>>& instances) {
	// Step 2: Bind the depth-stencil view and render target view to the output merger stage
	SP_DXGLRenderTargetView rtv = Engine::renderer()->getRTV(RESOURCE_VIEW_SLOT_BACK_BUFFER);
	ID3D11RenderTargetView* rtvv = rtv->get();
	SP_DXGLDepthStencilView dsv = Engine::renderer()->getDSV(RESOURCE_VIEW_SLOT_BACK_BUFFER);
	Engine::graphics()->context()->OMSetRenderTargets(1, &rtvv, dsv->get());

	// Step 3: Clear the render target buffer at the beginning of the render pass
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // Set your clear color
	// Engine::graphics()->context()->ClearRenderTargetView(rtvv, clearColor);
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
	Engine::graphics()->context()->VSSetShader(m_vertexShader->get(), nullptr, 0);
	Engine::graphics()->context()->PSSetShader(m_pixelShader->get(), nullptr, 0);

	// set up and bind view proj transform
	auto& cam = Engine::camera()->getActiveCamera();
	Transform t{};
	t.view = cam.view();
	t.proj = cam.proj();
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
	SP_InstanceBuffer instanceBuffer = Engine::resource()->createInstanceBuffer(&combinedInstances[0], combinedInstances.size(), sizeof(PerInstanceData));
	instanceBuffer->bind(1);

	// bind skybox and brdf
	Engine::resource()->get<SP_TextureCube>("space")->bind(1);
	m_brdf->bind(2);

	// bind lights
	Engine::renderer()->light()->getBuffer()->bind(1);

	int entityCount = 0;
	for (const auto& instance : instances) {
		const SP_Mesh& mesh = instance.first;
		// bind entity group data
		mesh->getMeshVertexBuffer()->bind(0);
		mesh->getIndexBuffer()->bind();

		// get sub meshes
		for (auto& subMesh : mesh->getMeshes()) {
			// set and bind material data
			SP_Material material = Engine::resource()->get<SP_Material>(subMesh.materialName);
			material->bind(0);

			MaterialId matId{};
			matId.textureIndex = mesh->getTextureIndex();
			matId.amountMetallic = mesh->amountMetallic();
			matId.amountRoughness = mesh->amountRoughness();
			matId.camPosition = cam.translation;
			m_pcbMaterial->update(&matId);
			m_pcbMaterial->bind(0);

			// draw sub meshes
			Engine::renderer()->drawIndexedTriangleListInstanced(subMesh.indexCount, instances[mesh].size(), subMesh.baseIndex, subMesh.baseVertex, entityCount);
		}
		entityCount += instances[mesh].size();
	}

	// reset/clean up
	Engine::graphics()->context()->OMSetRenderTargets(0, nullptr, nullptr);

	Engine::graphics()->context()->OMSetDepthStencilState(nullptr, 0);
	Engine::graphics()->context()->RSSetState(nullptr);

	Engine::graphics()->context()->VSSetShader(nullptr, nullptr, 0);
	Engine::graphics()->context()->PSSetShader(nullptr, nullptr, 0);
}
