#pragma once

#include <d3d11.h>
#include "DXGLMain.h"
#include "DXGLComponentTypes.h"

namespace dxgl {
	struct InstanceTransform {
		Vec3f scale{};
		Vec3f rotation{};
		Vec3f translation{};
	};

	class RenderPass {
	public:
		RenderPass();
		~RenderPass();

		virtual void draw(std::unordered_map<SP_Mesh, std::vector<InstanceTransform>>& instances) = 0;
	};
}
