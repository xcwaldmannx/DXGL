#pragma once

#include "DXGLMain.h"

#include "EarlyZRenderPass.h"
#include "LightingRenderPass.h"

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
		std::unordered_map<SP_Mesh, std::vector<Instance>> m_meshToInstances{};
		EarlyZRenderPass earlyZ{};
		LightingRenderPass lighting{};
	};
}
