#pragma once

#include "OctTree.h"
#include "OOBB.h"

#include "SkyboxRenderPass.h"
#include "EarlyZRenderPass.h"
#include "LightingRenderPass.h"
#include "MousePickRenderPass.h"
#include "CollisionRenderPass.h"
#include "HUDRenderPass.h"

#include "TextRenderManager.h"

namespace dxgl {
	class RenderQueue {
	public:
		RenderQueue();
		~RenderQueue();

		void submit(OctTree<governor::EntityId>::list entities);
		void submit(Text& text);
		void submit(const std::vector<OOBB>& oobbs);
		void draw();

	private:
		std::unordered_map<SP_Mesh, std::vector<PerInstanceData>> m_meshToInstances{};
		std::vector<Text> m_texts{};
		std::vector<Vec3f> m_oobbLines{};

		EarlyZRenderPass m_earlyZ{};
		LightingRenderPass m_lighting{};
		SkyboxRenderPass m_skybox{};
		MousePickRenderPass m_mousePick{};
		CollisionRenderPass m_collision{};
		HUDRenderPass m_hud{};
	};
}
