#pragma once

#include "DXGLDefinitions.h"
#include "Engine.h"

namespace dxgl {
	class CameraManager {
	public:
		CameraManager();
		~CameraManager();

		void update(long double delta);
		bool cullActiveCamera(governor::EntityId id);
		bool cullActiveCamera(Vec3f position, Vec3f scale, Vec3f min, Vec3f max);

		CameraComponent& getActiveCamera();
		void setActiveCamera(governor::EntityId id);

	private:
		std::vector<Vec4f> getFrustumPlanes();
	private:
		governor::EntityId m_activeCameraId = -1;
	};
}
