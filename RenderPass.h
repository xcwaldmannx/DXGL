#pragma once

#include <d3d11.h>

#include "EntityComponentTypes.h"

#include "DXGLRenderTargetView.h"
#include "DXGLDepthStencilView.h"

namespace dxgl {
	struct PerInstanceData {
		unsigned int id;
		Vec3f scale{};
		Vec3f rotation{};
		Vec3f translation{};
	};

	template<typename T>
	class RenderPass {
	public:
		RenderPass() {

		}

		~RenderPass() {

		}

		virtual void draw(T& instances) = 0;
	};
}
