#pragma once

#include "DXGLMain.h"

#include "OctTree.h"

#include "EarlyZRenderPass.h"
#include "LightingRenderPass.h"
#include "SkyboxRenderPass.h"

namespace dxgl {
	class RenderQueue {
	public:
		RenderQueue();
		~RenderQueue();

		void createInstances();
		void draw();

	private:
		governor::DXGLGroup* m_entities{};
		std::unordered_set<SP_Mesh> m_meshes{};
		std::unordered_map<SP_Mesh, std::vector<InstanceTransform>> m_meshToInstances{};

		OctTree<governor::EntityId> m_octTree{ OctTreeRect{}, 8 };
		bool init = false;

		EarlyZRenderPass m_earlyZ{};
		LightingRenderPass m_lighting{};
		SkyboxRenderPass m_skybox{};
	};
}
