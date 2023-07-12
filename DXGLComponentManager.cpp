#include "DXGLComponentManager.h"

using namespace dxgl::governor;

DXGLComponentManager::DXGLComponentManager() {

}

DXGLComponentManager::~DXGLComponentManager() {

}

void DXGLComponentManager::destroy(EntityId entityId) {
	m_entityComponents[entityId] = {};
}
