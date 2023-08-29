#include "DXGLGovernor.h"

using namespace dxgl::governor;

DXGLGovernor::DXGLGovernor() {
	m_componentManager = std::make_shared<DXGLComponentManager>();
	m_entityManager = std::make_shared<DXGLEntityManager>(m_componentManager);
}

DXGLGovernor::~DXGLGovernor() {
	for (auto& groupData : m_groups) {
		if (groupData.group) delete groupData.group;
	}
}

EntityId DXGLGovernor::createEntity() {
	EntityId entityId = m_entityManager->create();
	notifyGroupStateChange();
	executeOnCreateEntityCallbacks(entityId);
	return entityId;
}

void DXGLGovernor::destroyEntity(EntityId entityId) {
	executeOnDestroyEntityCallbacks(entityId);
	m_entityManager->destroy(entityId);
	notifyGroupStateChange();
}

const Signature& DXGLGovernor::getEntitySignature(EntityId entityId) {
	return m_entityManager->getSignature(entityId);
}

void DXGLGovernor::notifyGroupStateChange() {
	for (auto& group : m_groups) {
		DXGLGroup newGroup = m_entityManager->groupEntities(group.signature, group.sort);
		*(group.group) = newGroup;
	}
}

// callbacks
void DXGLGovernor::onCreateEntity(std::function<void(EntityId)> function) {
	m_callbacksOnCreateEntity.push_back(function);
}

void DXGLGovernor::onDestroyEntity(std::function<void(EntityId)> function) {
	m_callbacksOnDestroyEntity.push_back(function);
}

void DXGLGovernor::executeOnCreateEntityCallbacks(EntityId id) {
	for (auto& callback : m_callbacksOnCreateEntity) {
		callback(id);
	}
}

void DXGLGovernor::executeOnDestroyEntityCallbacks(EntityId id) {
	for (auto& callback : m_callbacksOnDestroyEntity) {
		callback(id);
	}
}

