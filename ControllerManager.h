#pragma once

#include "EntityComponentTypes.h"

namespace dxgl {
	class ControllerManager {
	public:
		ControllerManager();
		~ControllerManager();

		void update(long double delta);

		ControllerComponent& getActiveController();
		void setActiveController(governor::EntityId id);

	private:
		governor::EntityId m_activeControllerId = -1;

	};
}