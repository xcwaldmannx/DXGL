#pragma once

#include <vector>

#include "DXGLGovernorDefinitions.h"

namespace dxgl::governor {
	class DXGLEntityManager;

	class DXGLGroup : public std::vector<EntityId> {
	};
}