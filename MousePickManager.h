#pragma once

#include <cstdint>

#include <d3d11.h>

#include "DXGLDefinitions.h"
#include "DXGLMain.h"
#include "Point2f.h"

#include "DXGLComponentTypes.h"

namespace dxgl {
	class MousePickManager {
	public:
		MousePickManager();
		~MousePickManager();

		uint32_t getColorId(Point2f position);

	private:
		governor::DXGLGroup* m_entities = nullptr;
	};
}