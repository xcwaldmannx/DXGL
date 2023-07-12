#include "DXGLGovernor.h"

using namespace dxgl::governor;

DXGLGovernor::DXGLGovernor() {
	m_componentManager = std::make_shared<DXGLComponentManager>();
	m_entityManager = std::make_shared<DXGLEntityManager>(m_componentManager);
}

DXGLGovernor::~DXGLGovernor() {
	for (auto& groupData : m_groups) {
		delete groupData.group;
	}
}

EntityId DXGLGovernor::createEntity() {
	EntityId id = m_entityManager->create();
	notifyGroupStateChange();
	return id;
}

void DXGLGovernor::destroyEntity(EntityId entityId) {
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
