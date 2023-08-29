#pragma once

#include <string>
#include <unordered_map>

#include "DXGLDefinitions.h"
#include "Camera.h"

namespace dxgl {
	class CameraManager {
	public:
		CameraManager();
		~CameraManager();

		SP_Camera create(const std::string& alias);
		SP_Camera get(const std::string& alias);

	private:
		std::unordered_map<std::string, SP_Camera> m_cameras{};
	};
}
