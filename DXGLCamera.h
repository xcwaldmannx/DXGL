#pragma once

#include <vector>

#include "Mat4f.h"
#include "DXGLWindow.h"
#include "DXGLInputManager.h"

namespace dxgl {

	class DXGLCamera {
	public:
		DXGLCamera(DXGLWindow* window, SP_DXGLInputManager input);
		~DXGLCamera();

		void update(long double deltaTime);

		Mat4f& world();
		Mat4f& view();
		Mat4f& proj();

		Vec3f getPosition();
		Vec3f getDirection();

		bool cull(Vec3f position, Vec3f scale, Vec3f minVertex, Vec3f maxVertex);
		std::vector<Vec4f> getFrustum();

	private:
		DXGLWindow* m_window = nullptr;
		SP_DXGLInputManager m_input = nullptr;

		Mat4f m_world{};
		Mat4f m_view{};
		Mat4f m_proj{};

		float m_rotX = 0;
		float m_rotY = 0;

		Vec3f m_position{ 0, 0, 0 };
		Vec3f m_direction{ 0, 0, 0 };
	};

}
