#include "DXGLEntityManager.h"

#include <iostream>

using namespace dxgl::governor;

DXGLEntityManager::DXGLEntityManager(const std::shared_ptr<DXGLComponentManager>& componentManager) : m_componentManager(componentManager) {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		m_entities[i] = std::bitset<MAX_COMPONENTS>(0);
		m_availableEntities.push_back(i);
	}
}

DXGLEntityManager::~DXGLEntityManager() {

}

EntityId DXGLEntityManager::create() {
	assert(m_activeEntities.size() < MAX_ENTITIES && "Entity limit exceeded.");

	EntityId id = m_availableEntities.front();
	m_availableEntities.pop_front();
	m_activeEntities.push_back(id);
	return id;
}

void DXGLEntityManager::destroy(EntityId entityId) {
	m_entities[entityId].reset();
	m_componentManager->destroy(entityId);
	m_availableEntities.push_back(entityId);
	auto it = std::find(m_activeEntities.begin(), m_activeEntities.end(), entityId);
	m_activeEntities.erase(it);
}

const DXGLGroup DXGLEntityManager::groupEntities(const Signature& signature, GroupSort sort) {
	DXGLGroup group{};

	switch (sort) {
	case GROUP_ONLY:
		for (int i = 0; i < m_activeEntities.size(); i++) {
			uint32_t entityId = m_activeEntities[i];
			if (m_entities[entityId] == signature) {
				group.push_back(entityId);
			}
		}
		break;
	case GROUP_ANY:
		for (int i = 0; i < m_activeEntities.size(); i++) {
			uint32_t entityId = m_activeEntities[i];
			std::bitset bitsetAND = m_entities[entityId] & signature;
			if ((bitsetAND & signature) == signature) {
				group.push_back(entityId);
			}
		}
		break;
	case GROUP_NOT:
		for (int i = 0; i < m_activeEntities.size(); i++) {
			uint32_t entityId = m_activeEntities[i];
			if ((m_entities[entityId] & signature) == 0) {
				group.push_back(entityId);
			}
		}
		break;
	}

	return group;
}

const DXGLGroup DXGLEntityManager::groupEntities(std::vector<EntityId> entities, const Signature& signature, GroupSort sort) {
	DXGLGroup group{};

	switch (sort) {
	case GROUP_ONLY:
		for (int i = 0; i < entities.size(); i++) {
			uint32_t entityId = entities[i];
			if (m_entities[entityId] == signature) {
				group.push_back(entityId);
			}
		}
		break;
	case GROUP_ANY:
		for (int i = 0; i < entities.size(); i++) {
			uint32_t entityId = entities[i];
			std::bitset bitsetAND = m_entities[entityId] & signature;
			if ((bitsetAND & signature) == signature) {
				group.push_back(entityId);
			}
		}
		break;
	case GROUP_NOT:
		for (int i = 0; i < entities.size(); i++) {
			uint32_t entityId = entities[i];
			if ((m_entities[entityId] & signature) == 0) {
				group.push_back(entityId);
			}
		}
		break;
	}

	return group;
}

const Signature& DXGLEntityManager::getSignature(EntityId entityId) {
	return m_entities[entityId];
}
