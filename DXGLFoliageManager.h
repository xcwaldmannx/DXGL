#pragma once

#include "DXGLGovernor.h"

#include "Vec3f.h"

namespace dxgl {

	struct Foliage {
		Vec3f position{};
	};

	class DXGLFoliageManager {
	public:
		DXGLFoliageManager();
		~DXGLFoliageManager();

		void add(Foliage foliage);

		void update(long double delta);
		void draw();

	private:
		governor::DXGLGovernor governor;
	};
}