#include "RenderQueue.h"

#include "DXGLComponentTypes.h"

using namespace dxgl;

RenderQueue::RenderQueue() {
}

RenderQueue::~RenderQueue() {
}

void RenderQueue::submit(OctTree<governor::EntityId>::list entities) {
	auto& camera = Engine::camera()->getActiveCamera();

	for (auto entity : entities) {
		governor::EntityId id = entity->item;
		auto& transform = Engine::entities()->getEntityComponent<TransformComponent>(id);
		auto& mesh = Engine::entities()->getEntityComponent<MeshComponent>(id);

		if (!Engine::camera()->cullActiveCamera(id)) {
			m_meshToInstances[mesh.mesh].emplace_back(
				PerInstanceData {
					.id = id,
					.scale = transform.scale,
					.rotation = transform.rotation,
					.translation = transform.translation
				}
			);
		}
	}
}

void RenderQueue::submit(const std::vector<OOBB>& oobbs) {
	for (auto& OOBB : oobbs) {
		m_oobbLines.push_back(OOBB.m_center);
		m_oobbLines.push_back(OOBB.m_center + OOBB.m_axes[0]);
		m_oobbLines.push_back(OOBB.m_center);
		m_oobbLines.push_back(OOBB.m_center + OOBB.m_axes[1]);
		m_oobbLines.push_back(OOBB.m_center);
		m_oobbLines.push_back(OOBB.m_center + OOBB.m_axes[2]);

		m_oobbLines.push_back(OOBB.m_center);
		m_oobbLines.push_back(OOBB.m_center + Vec3f{ OOBB.m_extents.x, 0, 0 });
		m_oobbLines.push_back(OOBB.m_center);
		m_oobbLines.push_back(OOBB.m_center + Vec3f{ 0, OOBB.m_extents.y, 0 });
		m_oobbLines.push_back(OOBB.m_center);
		m_oobbLines.push_back(OOBB.m_center + Vec3f{ 0, 0, OOBB.m_extents.z });
	}
}


void RenderQueue::draw() {
	m_skybox.draw(m_meshToInstances);
	//m_earlyZ.draw(m_meshToInstances);
	m_lighting.draw(m_meshToInstances);
	m_mousePick.draw(m_meshToInstances);
	m_collision.draw(m_oobbLines);

	m_meshToInstances.clear();
	m_oobbLines.clear();
}
