#include "Skybox.h"

Skybox::Skybox() {
	// cubemaps
	m_desert = dxgl::DXGLMain::resource()->createTextureCube("Assets/Cubemaps/desert/");
	m_sky = dxgl::DXGLMain::resource()->createTextureCube("Assets/Cubemaps/Sky/");
	m_test = dxgl::DXGLMain::resource()->createTextureCube("Assets/Cubemaps/test/");

	// input layout
	dxgl::InputLayoutDesc descSkybox{};
	descSkybox.add("POSITION", 0, dxgl::FLOAT3, false);
	m_layout = dxgl::DXGLMain::resource()->createInputLayout(descSkybox, "Assets/Shaders/VS_Skybox.cso");

	// shaders
	m_vs = dxgl::DXGLMain::renderer()->shader()->create<dxgl::DXGLVertexShader>("Assets/Shaders/VS_Skybox.cso");
	m_ps = dxgl::DXGLMain::renderer()->shader()->create<dxgl::DXGLPixelShader>("Assets/Shaders/PS_Skybox.cso");

	// cbuffer
	m_cb = dxgl::DXGLMain::resource()->createCBuffer(sizeof(dxgl::TransformBuffer));

	// depth stencil
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dxgl::DXGLMain::renderer()->merger()->createDepthStencil(depthStencilDesc, "skybox");
}

Skybox::~Skybox() {

}

void Skybox::update(long double delta) {
}

void Skybox::draw() {
	dxgl::SP_DXGLCamera cam = dxgl::DXGLMain::renderer()->camera()->get("primary");

	dxgl::DXGLMain::renderer()->merger()->setDepthStencil("skybox");
	dxgl::DXGLMain::renderer()->shader()->VS_setShader(m_vs);
	dxgl::DXGLMain::renderer()->shader()->HS_setShader(nullptr);
	dxgl::DXGLMain::renderer()->shader()->DS_setShader(nullptr);
	dxgl::DXGLMain::renderer()->shader()->PS_setShader(m_ps);
	dxgl::TransformBuffer sbuff{};
	sbuff.world = cam->world();
	sbuff.view = cam->view();
	sbuff.proj = cam->proj();
	m_cb->update(&sbuff);
	dxgl::DXGLMain::renderer()->shader()->VS_setCBuffer(0, 1, m_cb->get());
	dxgl::DXGLMain::renderer()->shader()->PS_setResource(0, m_desert->get());

	dxgl::SP_DXGLMesh mesh = dxgl::DXGLMain::resource()->get<dxgl::SP_DXGLMesh>("sphereFlipped");

	dxgl::DXGLMain::renderer()->input()->setInputLayout(m_layout);
	dxgl::DXGLMain::renderer()->input()->setVertexBuffer(0, 1, &mesh->getVertexBuffer());
	dxgl::DXGLMain::renderer()->input()->setIndexBuffer(mesh->getIndexBuffer());
	dxgl::DXGLMain::renderer()->drawIndexedTriangleList(mesh->getIndices().size(), 0, 0);
}

dxgl::SP_DXGLTextureCube Skybox::getCube() {
	return m_desert;
}
