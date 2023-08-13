#include "RenderQueue.h"

#include "DXGLComponentTypes.h"

using namespace dxgl;

RenderQueue::RenderQueue() {
}

RenderQueue::~RenderQueue() {
}

void RenderQueue::submit(OctTree<governor::EntityId>::list entities) {
	m_meshes.clear();
	m_meshToInstances.clear();

	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");

	for (auto id : entities) {
		auto& transform = DXGLMain::entities()->getEntityComponent<TransformComponent>(*id);
		auto& mesh = DXGLMain::entities()->getEntityComponent<MeshComponent>(*id);

		if (!cam->cull(transform.translation, transform.scale, mesh.mesh->getAABB().min, mesh.mesh->getAABB().max)) {
			m_meshes.emplace(mesh.mesh);

			m_meshToInstances[mesh.mesh].emplace_back(
				PerInstanceData{
					.id = *id,
					.scale = transform.scale,
					.rotation = transform.rotation,
					.translation = transform.translation
				}
			);
		}
	}
}


void RenderQueue::draw() {
	m_skybox.draw(m_meshToInstances);
	//m_earlyZ.draw(m_meshToInstances);
	m_lighting.draw(m_meshToInstances);
	m_mousePick.draw(m_meshToInstances);
}
