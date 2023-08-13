#include "SkyboxRenderPass.h"

using namespace dxgl;

SkyboxRenderPass::SkyboxRenderPass() {
	// Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
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

	// Input layout
	dxgl::InputLayoutDesc descSkybox{};
	descSkybox.add("POSITION", 0, dxgl::FLOAT3, false);
	m_layout = dxgl::DXGLMain::resource()->createInputLayout(descSkybox, "Assets/Shaders/VS_Skybox.cso");

	// Shaders
	m_vertexShader = DXGLMain::resource()->createShader<DXGLVertexShader>("Assets/Shaders/VS_Skybox.cso");
	m_pixelShader = DXGLMain::resource()->createShader<DXGLPixelShader>("Assets/Shaders/PS_Skybox.cso");

	// Cubemap
	dxgl::DXGLMain::resource()->storeTextureCube("Assets/Cubemaps/space/", "space");
	m_skybox = dxgl::DXGLMain::resource()->get<dxgl::SP_TextureCube>("space");

	// transform buffer
	m_vcbTransform = DXGLMain::resource()->createVSConstantBuffer(sizeof(Transform));
}

SkyboxRenderPass::~SkyboxRenderPass() {

}

void SkyboxRenderPass::draw(std::unordered_map<SP_Mesh, std::vector<PerInstanceData>>& instances) {
	// Bind the depth-stencil view and render target view
	SP_DXGLRenderTargetView rtv = DXGLMain::renderer()->getRTV(RESOURCE_VIEW_SLOT_BACK_BUFFER);
	ID3D11RenderTargetView* rtvv = rtv->get();
	SP_DXGLDepthStencilView dsv = DXGLMain::renderer()->getDSV(RESOURCE_VIEW_SLOT_BACK_BUFFER);
	DXGLMain::graphics()->context()->OMSetRenderTargets(1, &rtvv, dsv->get());

	// Step 3: Clear the render target buffer at the beginning of the render pass
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // Set your clear color
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
	DXGLMain::graphics()->context()->VSSetShader(m_vertexShader->get(), nullptr, 0);
	DXGLMain::graphics()->context()->PSSetShader(m_pixelShader->get(), nullptr, 0);

	// set up and bind view proj transform
	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");
	Transform t{};
	t.world = cam->world();
	t.view = cam->view();
	t.proj = cam->proj();
	m_vcbTransform->update(&t);
	m_vcbTransform->bind(0);

	// bind cubemap texture
	m_skybox->bind(0);

	// draw skybox onto mesh
	dxgl::SP_Mesh mesh = dxgl::DXGLMain::resource()->get<dxgl::SP_Mesh>("cubeFlipped");

	mesh->getMeshVertexBuffer()->bind(0);
	mesh->getIndexBuffer()->bind();

	for (auto& subMesh : mesh->getMeshes()) {
		dxgl::DXGLMain::renderer()->drawIndexedTriangleList(subMesh.indexCount, subMesh.baseIndex, subMesh.baseVertex);
	}
}
