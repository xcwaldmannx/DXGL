#include "ControllerManager.h"

#include "Engine.h"
#include "InputManager.h"
#include "EntityManager.h"

using namespace dxgl;

ControllerManager::ControllerManager() {

}

ControllerManager::~ControllerManager() {

}

void ControllerManager::update(long double delta) {
	if (m_activeControllerId == -1) return;

	auto& controller = Engine::entities()->getEntityComponent<ControllerComponent>(m_activeControllerId);
	for (auto& action : controller.actions) {
		switch (action.flag) {
		case ControllerActionFlag::ON_PRESS:
			if (Engine::input()->getKeyPressState(action.key)) {
				action.function(m_activeControllerId, delta);
			}
			break;
		case ControllerActionFlag::ON_HOLD:
			if (Engine::input()->getKeyHoldState(action.key, 200)) {
				action.function(m_activeControllerId, delta);
			}
			break;
		case ControllerActionFlag::ON_TAP:
			if (Engine::input()->getKeyTapState(action.key)) {
				action.function(m_activeControllerId, delta);
			}
			break;
		case ControllerActionFlag::ON_RELEASE:
			if (Engine::input()->getKeyReleaseState(action.key)) {
				action.function(m_activeControllerId, delta);
			}
			break;
		}
	}
}

ControllerComponent& ControllerManager::getActiveController() {
	assert(m_activeControllerId != -1 && "No controller is currently active. Could not return ControllerComponent.");
	return Engine::entities()->getEntityComponent<ControllerComponent>(m_activeControllerId);
}

void ControllerManager::setActiveController(governor::EntityId id) {
	if (Engine::entities()->entityHasComponent<ControllerComponent>(id)) {

		if (m_activeControllerId != -1) {
			auto& controller = Engine::entities()->getEntityComponent<ControllerComponent>(m_activeControllerId);
			controller.isActive = false;
		}

		auto& controller = Engine::entities()->getEntityComponent<ControllerComponent>(id);
		m_activeControllerId = id;
		controller.isActive = true;
	}
}