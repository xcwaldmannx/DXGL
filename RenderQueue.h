#pragma once

#include "DXGLMain.h"

#include "OctTree.h"

#include "SkyboxRenderPass.h"
#include "EarlyZRenderPass.h"
#include "LightingRenderPass.h"
#include "MousePickRenderPass.h"

namespace dxgl {
	class RenderQueue {
	public:
		RenderQueue();
		~RenderQueue();

		void submit(OctTree<governor::EntityId>::list entities);
		void draw();

	private:
		governor::DXGLGroup* m_entities{};
		std::unordered_set<SP_Mesh> m_meshes{};
		std::unordered_map<SP_Mesh, std::vector<PerInstanceData>> m_meshToInstances{};

		OctTree<governor::EntityId> m_octTree{ OctTreeRect{}, 8 };
		bool init = false;

		EarlyZRenderPass m_earlyZ{};
		LightingRenderPass m_lighting{};
		SkyboxRenderPass m_skybox{};
		MousePickRenderPass m_mousePick{};
	};
}
