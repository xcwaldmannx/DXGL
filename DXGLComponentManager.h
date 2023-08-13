#pragma once

#include <cstdint>
#include <typeindex>
#include <cassert>
#include <unordered_map>
#include <memory>
#include <initializer_list>

#include "DXGLGovernorDefinitions.h"
#include "DXGLComponentList.h"

namespace dxgl::governor {

	class DXGLComponentManager {
	public:
		DXGLComponentManager();
		~DXGLComponentManager();

		template<typename T>
		void registerComponent() {
			assert(m_registeredComponentCount < MAX_COMPONENTS && "Component registration limit exceeded.");
			assert(!exists<T>() && "Component already registered.");

			std::type_index componentType = typeid(T);
			m_registeredComponents[componentType] = m_registeredComponentCount;
			m_registeredComponentCount++;
		}

		template<typename T>
		void add(T& component, EntityId entityId, Signature& entitySignature) {
			assert(exists<T>() && "Component does not exist.");

			std::type_index componentType = typeid(T);
			ComponentId componentId = m_registeredComponents[componentType];
			entitySignature.set(componentId);
			m_entityComponents[entityId].add<T>(component, componentId);
		}

		template<typename T>
		T& get(EntityId entityId) {
			assert(exists<T>() && "Component does not exist.");

			std::type_index componentType = typeid(T);
			ComponentId componentId = m_registeredComponents[componentType];
			return m_entityComponents[entityId].get<T>(componentId);
		}

		template<typename T>
		T remove(EntityId entityId, Signature& entitySignature) {
			assert(exists<T>() && "Component does not exist.");

			std::type_index componentType = typeid(T);
			ComponentId componentId = m_registeredComponents[componentType];
			entitySignature.reset(componentId);
			return m_entityComponents[entityId].remove<T>(componentId);
		}

		template<typename... T>
		Signature getSignature() {
			std::vector<ComponentId> ids{};

			Signature signature{};

			((signature.set(m_registeredComponents[typeid(T)])), ...);

			return signature;
		}

		void destroy(EntityId entityId);

	private:
		template<typename T>
		bool exists() {
			std::type_index componentType = typeid(T);
			auto iterator = m_registeredComponents.find(componentType);
			return iterator != m_registeredComponents.end();
		}

	private:
		uint32_t m_registeredComponentCount = 0;

		// Registered components: <Class Type, Component ID>
		std::unordered_map<std::type_index, ComponentId> m_registeredComponents{};

		// List of entity's components: <Entity ID, ComponentList>
		std::unordered_map<EntityId, DXGLComponentList> m_entityComponents{};
	};
}