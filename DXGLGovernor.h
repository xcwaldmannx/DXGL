#pragma once

#include <memory>
#include <iostream>
#include <functional>

#include "DXGLGovernorDefinitions.h"

#include "DXGLEntityManager.h"
#include "DXGLComponentManager.h"
#include "DXGLGroup.h"

namespace dxgl::governor {
	struct GroupData {
		DXGLGroup* group = nullptr;
		GroupSort sort;
		Signature signature{};
	};

	class DXGLGovernor {
	public:
		DXGLGovernor();
		~DXGLGovernor();

		EntityId createEntity();

		void destroyEntity(EntityId entityId);

		template<typename T>
		void registerComponent() {
			m_componentManager->registerComponent<T>();
		}

		template<typename T>
		void addEntityComponent(T& component, EntityId entityId) {
			m_entityManager->addComponent<T>(component, entityId);
			notifyGroupStateChange();
			executeOnAddComponentCallbacks<T>(entityId);
		}

		template<typename T>
		T& getEntityComponent(EntityId entityId) {
			return m_entityManager->getComponent<T>(entityId);
		}

		template<typename T>
		T removeEntityComponent(EntityId entityId) {
			T component = m_entityManager->removeComponent<T>(entityId);
			notifyGroupStateChange();
			executeOnRemoveComponentCallbacks<T>(entityId);
			return component;
		}

		template<typename T>
		bool entityHasComponent(EntityId entityId) {
			return m_entityManager->hasComponent<T>(entityId);
		}

		const Signature& getEntitySignature(EntityId entityId);

		template<typename... T>
		void group(GroupSort sort, DXGLGroup*& group) {

			Signature signature = m_componentManager->getSignature<T...>();

			// Check if the group already exists
			bool exists = false;
			GroupData existingGroup{};
			for (auto& groupData : m_groups) {
				if (groupData.signature == signature) {
					existingGroup = groupData;
					exists = true;
					break;
				}
			}

			if (!exists) {
				// Group does not exist, create a new one
				GroupData groupData{};
				groupData.group = new DXGLGroup(m_entityManager->groupEntities(signature, sort));
				groupData.sort = sort;
				groupData.signature = signature;

				m_groups.push_back(groupData);
				m_groupSignatures.push_back(signature);

				group = groupData.group;
			} else {
				// Group exists, assign the existing group to the provided object
				group = existingGroup.group;
			}
		}

		template<typename... T>
		DXGLGroup collectAllWith(DXGLGroup* group) {
			DXGLGroup newGroup;
			for (governor::EntityId entityId : *group) {
				if (m_entityManager->hasComponent<T>(entityId)) {
					newGroup.push_back(entityId);
				}
			}
			return newGroup;
		}

		// callbacks
		void onCreateEntity(std::function<void(EntityId)> function);
		void onDestroyEntity(std::function<void(EntityId)> function);

		template<typename T>
		void onAddComponent(std::function<void(EntityId)> function) {
			m_callbacksOnAddComponent[typeid(T)].push_back(function);
		}

		template<typename T>
		void onRemoveComponent(std::function<void(EntityId)> function) {
			m_callbacksOnRemoveComponent[typeid(T)].push_back(function);
		}

	private:
		void executeOnCreateEntityCallbacks(EntityId id);
		void executeOnDestroyEntityCallbacks(EntityId id);

		template<typename T>
		void executeOnAddComponentCallbacks(EntityId id) {
			auto& callbacks = m_callbacksOnAddComponent[typeid(T)];
			for (auto& callback : callbacks) {
				callback(id);
			}
		}

		template<typename T>
		void executeOnRemoveComponentCallbacks(EntityId id) {
			auto& callbacks = m_callbacksOnRemoveComponent[typeid(T)];
			for (auto& callback : callbacks) {
				callback(id);
			}
		}

	private:
		void notifyGroupStateChange();

	private:
		std::shared_ptr<DXGLEntityManager> m_entityManager = nullptr;
		std::shared_ptr<DXGLComponentManager> m_componentManager = nullptr;

		std::vector<GroupData> m_groups{};
		std::vector<Signature> m_groupSignatures{};

		// callbacks
		std::vector<std::function<void(EntityId)>> m_callbacksOnCreateEntity{};
		std::vector<std::function<void(EntityId)>> m_callbacksOnDestroyEntity{};

		std::unordered_map<std::type_index, std::vector<std::function<void(EntityId)>>> m_callbacksOnAddComponent{};
		std::unordered_map<std::type_index, std::vector<std::function<void(EntityId)>>> m_callbacksOnRemoveComponent{};
	};
}