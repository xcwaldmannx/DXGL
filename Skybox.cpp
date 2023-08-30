#include "Skybox.h"

#include "Engine.h"
#include "ResourceManager.h"
#include "CameraManager.h"

Skybox::Skybox() {
	// cubemaps
	dxgl::Engine::resource()->storeTextureCube("Assets/Cubemaps/desert/", "desert");
	m_desert = dxgl::Engine::resource()->get<dxgl::SP_TextureCube>("desert");

	dxgl::Engine::resource()->storeTextureCube("Assets/Cubemaps/space/", "space");
	m_space = dxgl::Engine::resource()->get<dxgl::SP_TextureCube>("space");

	m_sky = dxgl::Engine::resource()->createTextureCube("Assets/Cubemaps/Sky/");
	m_test = dxgl::Engine::resource()->createTextureCube("Assets/Cubemaps/test/");

	// input layout
	dxgl::InputLayoutDesc descSkybox{};
	descSkybox.add("POSITION", 0, dxgl::FLOAT3, false);
	m_layout = dxgl::Engine::resource()->createInputLayout(descSkybox, "Assets/Shaders/VS_Skybox.cso");

	// shaders
	m_vs = dxgl::Engine::renderer()->shader()->create<dxgl::DXGLVertexShader>("Assets/Shaders/VS_Skybox.cso");
	m_ps = dxgl::Engine::renderer()->shader()->create<dxgl::DXGLPixelShader>("Assets/Shaders/PS_Skybox.cso");

	// cbuffer
	m_vscb = dxgl::Engine::resource()->createVSConstantBuffer(sizeof(dxgl::TransformBuffer));

	// depth stencil
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dxgl::Engine::renderer()->merger()->createDepthStencil(depthStencilDesc, "skybox");
}

Skybox::~Skybox() {

}

void Skybox::update(long double delta) {
}

void Skybox::draw() {
	auto& cam = dxgl::Engine::camera()->getActiveCamera();

	dxgl::Engine::renderer()->merger()->setDepthStencil("skybox");
	dxgl::Engine::renderer()->shader()->VS_setShader(m_vs);
	dxgl::Engine::renderer()->shader()->HS_setShader(nullptr);
	dxgl::Engine::renderer()->shader()->DS_setShader(nullptr);
	dxgl::Engine::renderer()->shader()->PS_setShader(m_ps);
	dxgl::TransformBuffer sbuff{};
	sbuff.world = cam.world();
	sbuff.view = cam.view();
	sbuff.proj = cam.proj();
	m_vscb->update(&sbuff);
	m_vscb->bind(0);

	dxgl::Engine::renderer()->shader()->PS_setResource(0, m_space->get());

	dxgl::SP_Mesh mesh = dxgl::Engine::resource()->get<dxgl::SP_Mesh>("cubeFlipped");

	m_layout->bind();
	mesh->getMeshVertexBuffer()->bind(0);
	mesh->getIndexBuffer()->bind();

	for (auto& subMesh : mesh->getMeshes()) {
		dxgl::Engine::renderer()->drawIndexedTriangleList(subMesh.indexCount, subMesh.baseIndex, subMesh.baseVertex);
	}
}

dxgl::SP_TextureCube Skybox::getCube() {
	return m_desert;
}
