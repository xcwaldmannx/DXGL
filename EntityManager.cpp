#include "EntityManager.h"

using namespace dxgl;

EntityManager::EntityManager() {
	OctTreeRect rect{};
	Vec3f size{ 6400, 6400, 6400 };
	rect.pos = size * -1;
	rect.size = size * 2;
	m_entities.resize(rect);
}

EntityManager::~EntityManager() {

}

std::list<OctTreeItem<governor::EntityId>> EntityManager::getEntities() {
	return m_entities.allItems();
}

OctTree<governor::EntityId>::list EntityManager::searchEntities(float searchSize) {
	auto& cam = Engine::camera()->getActiveCamera();

	OctTreeRect searchArea{};
	Vec3f size{ searchSize, searchSize, searchSize };
	searchArea.pos = cam.translation - size;
	searchArea.size = size * 2;
	return m_entities.search(searchArea);
}

governor::EntityId EntityManager::createEntity(TransformComponent& transform, MeshComponent& mesh) {
	governor::EntityId id = m_governor.createEntity();

	addEntityComponent<TransformComponent>(transform, id);
	addEntityComponent<MeshComponent>(mesh, id);

	OctTreeRect rect{};
	rect.pos = transform.translation + mesh.mesh->getAABB().min;
	rect.size = mesh.mesh->getAABB().max;
	m_entities.insert(id, rect);

	return id;
}

void EntityManager::destroyEntity(governor::EntityId id) {
	m_governor.destroyEntity(id);
	m_entities.remove(id);
}

void EntityManager::relocateEntity(governor::EntityId id) {
	auto& transform = getEntityComponent<TransformComponent>(id);
	auto& mesh = getEntityComponent<MeshComponent>(id);
	OctTreeRect rect{};
	rect.pos = transform.translation + mesh.mesh->getAABB().min;
	rect.size = mesh.mesh->getAABB().max;
	m_entities.relocate(id, rect);
}

// callbacks
void EntityManager::onCreateEntity(std::function<void(governor::EntityId)> function) {
	m_governor.onCreateEntity(function);
}

void EntityManager::onDestroyEntity(std::function<void(governor::EntityId)> function) {
	m_governor.onDestroyEntity(function);
}
