#include "RenderQueue.h"

#include "DXGLComponentTypes.h"

using namespace dxgl;

RenderQueue::RenderQueue() {
	DXGLMain::governor()->group<TransformComponent, MeshComponent>(governor::GROUP_ANY, m_entities);
}

RenderQueue::~RenderQueue() {

}

void RenderQueue::createInstances() {
	m_meshes.clear();
	m_meshToInstances.clear();

	// track meshes and sort instances
	for (auto id : *m_entities) {
		auto& mesh = DXGLMain::governor()->getEntityComponent<MeshComponent>(id);
		auto& transform = DXGLMain::governor()->getEntityComponent<TransformComponent>(id);

		SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");
		if (!cam->cull(transform.translation, transform.scale, mesh.mesh->getAABB().min, mesh.mesh->getAABB().max)) {
			m_meshes.emplace(mesh.mesh);

			m_meshToInstances[mesh.mesh].emplace_back(
				Instance{
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
	earlyZ.draw(m_meshToInstances);
	lighting.draw(m_meshToInstances);
}
