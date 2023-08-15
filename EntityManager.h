#pragma once


#include "DXGLDefinitions.h"
#include "DXGLComponentTypes.h"

#include "DXGLGovernor.h"
#include "OctTree.h"

#include "Mesh.h"

namespace dxgl {
	class EntityManager {
	public:
		EntityManager();
		~EntityManager();

		std::list<OctTreeItem<governor::EntityId>> getEntities();
		OctTree<governor::EntityId>::list searchEntities(float searchSize);

		template<typename T>
		void registerComponent() {
			m_governor.registerComponent<T>();
		}

		governor::EntityId createEntity(TransformComponent& transform, MeshComponent& mesh);

		void destroyEntity(governor::EntityId id);

		void relocateEntity(governor::EntityId id);

		template<typename T>
		void addEntityComponent(T& component, governor::EntityId id) {
			m_governor.addEntityComponent<T>(component, id);
		}

		template<typename T>
		T& getEntityComponent(governor::EntityId id) {
			return m_governor.getEntityComponent<T>(id);
		}

		template<typename T>
		T removeEntityComponent(governor::EntityId id) {
			return m_governor.removeEntityComponent<T>(id);
		}

		template<typename T>
		bool entityHasComponent(governor::EntityId id) {
			return m_governor.entityHasComponent<T>(id);
		}

		template<typename... T>
		void group(governor::GroupSort sort, governor::DXGLGroup*& group) {
			m_governor.group<T...>(sort, group);
		}

		template<typename... T>
		governor::DXGLGroup collectAllWith(governor::DXGLGroup* group) {
			return m_governor.collectAllWith<T...>(group);
		}

	private:
		governor::DXGLGovernor m_governor;
		OctTree<governor::EntityId> m_entities{ OctTreeRect{}, 12 };
	};
}