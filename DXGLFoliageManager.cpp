#include "DXGLFoliageManager.h"

using namespace dxgl;

DXGLFoliageManager::DXGLFoliageManager() {
	InputLayoutDesc desc{};
	desc.add("POSITION", 0, FLOAT3, false);
	desc.add("TEXCOORD", 0, FLOAT2, false);
	desc.add("NORMAL",   0, FLOAT3, false);
	desc.add("INSTANCE_SCALE",        1, FLOAT3, true);
	desc.add("INSTANCE_ROTATION",     1, FLOAT3, true);
	desc.add("INSTANCE_TRANSLATION",  1, FLOAT3, true);

	desc.add("INSTANCE_COLOR_ZERO",  1, FLOAT3, true);
	desc.add("INSTANCE_COLOR_ONE",   1, FLOAT3, true);
	desc.add("INSTANCE_COLOR_TWO",   1, FLOAT3, true);
	desc.add("INSTANCE_COLOR_THREE", 1, FLOAT3, true);

	desc.add("INSTANCE_TIME_OFFSET", 1, FLOAT1, true);
	m_layout = DXGLMain::resource()->createInputLayout(desc, "Assets/Shaders/VS_FoliageShader.cso");
	m_vs = DXGLMain::resource()->createShader<DXGLVertexShader>("Assets/Shaders/VS_FoliageShader.cso");
	m_ps = DXGLMain::resource()->createShader<DXGLPixelShader>("Assets/Shaders/PS_FoliageShader.cso");

	m_cb = DXGLMain::resource()->createCBuffer(sizeof(FoliageBuffer));

	MeshDesc meshDesc{};
	meshDesc.vertexAttributes = VERTEX_POSITION | VERTEX_TEXCOORD | VERTEX_NORMAL;
	meshDesc.miscAttributes = MISC_INDEX;
	m_mesh = DXGLMain::resource()->createBasicMesh(meshDesc, "Assets/Meshes/landscapes/grass.fbx");
}

DXGLFoliageManager::~DXGLFoliageManager() {
}

void DXGLFoliageManager::add(FoliageInstance foliage) {
	m_foliage.push_back(foliage);
}

void DXGLFoliageManager::update(long double delta) {
	cull();

	m_timePassed += delta;

	FoliageBuffer buffer{};
	buffer.model.setIdentity();
	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");
	buffer.view = cam->view();
	buffer.proj = cam->proj();
	buffer.time = m_timePassed;
	m_cb->update(&buffer);
}

void DXGLFoliageManager::draw() {
	if (!m_mesh) return;
	DXGLMain::renderer()->input()->setInputLayout(m_layout);
	DXGLMain::renderer()->input()->setVertexBuffer(0, 1, &m_mesh->getMeshVertexBuffer());
	DXGLMain::renderer()->input()->setIndexBuffer(m_mesh->getIndexBuffer());

	if (!m_foliage.empty()) {
		DXGLMain::renderer()->input()->setVertexBuffer(1, 1, &m_vbInstance);
	}

	DXGLMain::renderer()->shader()->VS_setShader(m_vs);
	DXGLMain::renderer()->shader()->PS_setShader(m_ps);

	DXGLMain::renderer()->shader()->VS_setCBuffer(0, 1, m_cb->get());

	for (auto& mesh : m_mesh->getMeshes()) {
		DXGLMain::renderer()->drawIndexedTriangleListInstanced(mesh.indexCount, m_foliage.size(), mesh.baseIndex, mesh.baseVertex, 0);
	}
}

void DXGLFoliageManager::cull() {
	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");

	//m_culledFoliage.clear();

	//for (auto& foliage : m_foliage) {
	//	if (!cam->cull(foliage.translation, foliage.scale, m_mesh->getAABB().min, m_mesh->getAABB().max)) {
	//		m_culledFoliage.push_back(foliage);
	//	}
	//}

	if (!m_foliage.empty()) {
		m_vbInstance = DXGLMain::resource()->createVertexBuffer(&m_foliage[0], m_foliage.size(), sizeof(FoliageInstance));
	}
}
