#include "SkyboxRenderPass.h"

#include "Engine.h"
#include "Graphics.h"
#include "ResourceManager.h"
#include "CameraManager.h"

using namespace dxgl;

SkyboxRenderPass::SkyboxRenderPass() {
	// Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
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

	// Input layout
	dxgl::InputLayoutDesc descSkybox{};
	descSkybox.add("POSITION", 0, dxgl::FLOAT3, false);
	m_layout = dxgl::Engine::resource()->createInputLayout(descSkybox, "Assets/Shaders/VS_Skybox.cso");

	// Shaders
	m_vertexShader = Engine::resource()->createShader<DXGLVertexShader>("Assets/Shaders/VS_Skybox.cso");
	m_pixelShader = Engine::resource()->createShader<DXGLPixelShader>("Assets/Shaders/PS_Skybox.cso");

	// Cubemap
	dxgl::Engine::resource()->storeTextureCube("Assets/Cubemaps/space/", "space");
	m_skybox = dxgl::Engine::resource()->get<dxgl::SP_TextureCube>("space");

	// transform buffer
	m_vcbTransform = Engine::resource()->createVSConstantBuffer(sizeof(Transform));
}

SkyboxRenderPass::~SkyboxRenderPass() {

}

void SkyboxRenderPass::draw(std::unordered_map<SP_Mesh, std::vector<PerInstanceData>>& instances) {
	// Bind the depth-stencil view and render target view
	SP_DXGLRenderTargetView rtv = Engine::renderer()->getRTV(RESOURCE_VIEW_SLOT_BACK_BUFFER);
	ID3D11RenderTargetView* rtvv = rtv->get();
	SP_DXGLDepthStencilView dsv = Engine::renderer()->getDSV(RESOURCE_VIEW_SLOT_BACK_BUFFER);
	Engine::graphics()->context()->OMSetRenderTargets(1, &rtvv, dsv->get());

	// Step 3: Clear the render target buffer at the beginning of the render pass
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // Set your clear color
	Engine::graphics()->context()->ClearRenderTargetView(rtvv, clearColor);
	Engine::graphics()->context()->ClearDepthStencilView(dsv->get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

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
	t.world = cam.world();
	t.view = cam.view();
	t.proj = cam.proj();
	m_vcbTransform->update(&t);
	m_vcbTransform->bind(0);

	// bind cubemap texture
	m_skybox->bind(0);

	// draw skybox onto mesh
	dxgl::SP_Mesh mesh = dxgl::Engine::resource()->get<dxgl::SP_Mesh>("cubeFlipped");

	mesh->getMeshVertexBuffer()->bind(0);
	mesh->getIndexBuffer()->bind();

	for (auto& subMesh : mesh->getMeshes()) {
		dxgl::Engine::renderer()->drawIndexedTriangleList(subMesh.indexCount, subMesh.baseIndex, subMesh.baseVertex);
	}
}
