#include "RenderQueue.h"

#include "DXGLComponentTypes.h"

using namespace dxgl;

RenderQueue::RenderQueue() {
	DXGLMain::governor()->group<TransformComponent, MeshComponent>(governor::GROUP_ANY, m_entities);

	OctTreeRect rect{};
	Vec3f size{ 6400, 6400, 6400 };
	rect.pos = size * -1;
	rect.size = size * 2;
	m_octTree.resize(rect);
}

RenderQueue::~RenderQueue() {

}

void RenderQueue::createInstances() {
	m_meshes.clear();
	m_meshToInstances.clear();

	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");

	// track meshes and sort instances
	if (!init) {
		for (auto id : *m_entities) {
			auto& mesh = DXGLMain::governor()->getEntityComponent<MeshComponent>(id);
			auto& transform = DXGLMain::governor()->getEntityComponent<TransformComponent>(id);

			OctTreeRect rect{};
			rect.pos = transform.translation + mesh.mesh->getAABB().min;
			rect.size = mesh.mesh->getAABB().max;
			m_octTree.insert(id, rect);
		}
		init = true;
	}

	OctTreeRect searchArea{};
	Vec3f size{ 1024, 1024, 1024 };
	searchArea.pos = cam->getPosition() - size;
	searchArea.size = size * 2;

	for (auto id : m_octTree.search(searchArea)) {
		auto& mesh = DXGLMain::governor()->getEntityComponent<MeshComponent>(*id);
		auto& transform = DXGLMain::governor()->getEntityComponent<TransformComponent>(*id);

		if (!cam->cull(transform.translation, transform.scale, mesh.mesh->getAABB().min, mesh.mesh->getAABB().max)) {
			m_meshes.emplace(mesh.mesh);

			m_meshToInstances[mesh.mesh].emplace_back(
				InstanceTransform{
					.scale = transform.scale,
					.rotation = transform.rotation,
					.translation = transform.translation
				}
			);
		}
	}
}

void RenderQueue::draw() {
	createInstances();
	m_skybox.draw(m_meshToInstances);
	//m_earlyZ.draw(m_meshToInstances);
	m_lighting.draw(m_meshToInstances);
}
