#pragma once

#include <bitset>
#include <array>
#include <cassert>
#include <memory>
#include <deque>

#include "DXGLGovernorDefinitions.h"
#include "DXGLComponentManager.h"
#include "DXGLGroup.h"

namespace dxgl::governor {

	class DXGLEntityManager {
	public:
		DXGLEntityManager(const std::shared_ptr<DXGLComponentManager>& componentManager);
		~DXGLEntityManager();

		EntityId create();
		void destroy(EntityId entityId);

		template<typename T>
		void addComponent(T& component, EntityId entityId) {
			m_componentManager->add<T>(component, entityId, m_entities[entityId]);
		}

		template<typename T>
		T& getComponent(EntityId entityId) {
			return m_componentManager->get<T>(entityId);
		}

		template<typename T>
		T removeComponent(EntityId entityId) {
			return m_componentManager->remove<T>(entityId, m_entities[entityId]);
		}

		template<typename T>
		bool hasComponent(EntityId entityId) {
			auto it = std::find(m_activeEntities.begin(), m_activeEntities.end(), entityId);
			if (it == m_activeEntities.end()) {
				return false;
			}

			const Signature& search = m_componentManager->getSignature<T>();
			const Signature& test = getSignature(entityId);

			for (int i = 0; i < MAX_COMPONENTS; i++) {
				if (search[i]) {
					if (!test[i]) return false;
				}
			}
			
			return true;
		}

		const DXGLGroup groupEntities(const Signature& signature, GroupSort sort);
		const DXGLGroup groupEntities(std::vector<EntityId> entities, const Signature& signature, GroupSort sort);

		const Signature& getSignature(EntityId entityId);

	private:
		std::array<Signature, MAX_ENTITIES> m_entities{};
		std::deque<EntityId> m_availableEntities{};
		std::vector<EntityId> m_activeEntities{};

		std::shared_ptr<DXGLComponentManager> m_componentManager = nullptr;
	};
}