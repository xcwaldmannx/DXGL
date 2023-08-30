#pragma once

#include "DXGLDefinitions.h"

#include "Vec3f.h"

namespace dxgl {
	struct alignas(16) Light {
		Vec3f position{};
		float pad0 = 0;
		Vec3f direction{};
		float pad1 = 0;
		Vec3f color{};
		float pad2 = 0;
 		Vec2f angles{};
		float pad3[2] = {0, 0};
	};

	struct alignas(16) LightBuffer {
		Light lights[16];
		int lightCount;
		float pad[3];
	};

	class DXGLLight {
	public:
		DXGLLight();
		~DXGLLight();

		void addLight(Light& light);
		SP_PSConstantBuffer getBuffer();

	private:
		SP_PSConstantBuffer m_pscbLight = nullptr;
		int m_lightCount = 0;
		LightBuffer m_lightBuffer{};
	};
};