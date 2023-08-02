#pragma once

#include <string>
#include <unordered_map>

#include "DXGLCamera.h"

namespace dxgl {
	class DXGLCameraManager {
	public:
		DXGLCameraManager();
		~DXGLCameraManager();

		SP_DXGLCamera create(const std::string& alias);
		SP_DXGLCamera get(const std::string& alias);

	private:
		std::unordered_map<std::string, SP_DXGLCamera> m_cameras{};
	};
}
