#pragma once

#include <any>
#include <typeindex>
#include <cassert>
#include <array>

#include "DXGLGovernorDefinitions.h"

namespace dxgl::governor {

	class DXGLComponentList {
	public:
		DXGLComponentList();
		~DXGLComponentList();

		template<typename T>
		void add(T& component, ComponentId componentId) {
			std::type_index type = m_components[componentId].type();
			assert(type == typeid(int) && "Component is already set.");

			m_components[componentId] = component;
		}

		template<typename T>
		T& get(ComponentId componentId) {
			std::type_index type = m_components[componentId].type();
			assert(type != typeid(int) && "Component is not set.");

			return std::any_cast<T&>(m_components[componentId]);
		}

		template<typename T>
		T remove(ComponentId componentId) {
			std::type_index type = m_components[componentId].type();
			assert(type != typeid(int) && "Component is not set.");

			T component = m_components[componentId];
			m_components[componentId] = NULL_COMPONENT;
			return component;
		}

	private:
		std::array<std::any, MAX_COMPONENTS> m_components{};
	};
}