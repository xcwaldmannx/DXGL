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
	m_vscb = dxgl::DXGLMain::resource()->createVSConstantBuffer(sizeof(dxgl::TransformBuffer));

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
	m_vscb->update(&sbuff);
	m_vscb->bind(0);

	dxgl::DXGLMain::renderer()->shader()->PS_setResource(0, m_desert->get());

	dxgl::SP_Mesh mesh = dxgl::DXGLMain::resource()->get<dxgl::SP_Mesh>("cubeFlipped");

	m_layout->bind();
	mesh->getMeshVertexBuffer()->bind(0);
	mesh->getIndexBuffer()->bind();

	for (auto& subMesh : mesh->getMeshes()) {
		dxgl::DXGLMain::renderer()->drawIndexedTriangleList(subMesh.indexCount, subMesh.baseIndex, subMesh.baseVertex);
	}
}

dxgl::SP_TextureCube Skybox::getCube() {
	return m_desert;
}
