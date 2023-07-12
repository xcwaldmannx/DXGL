#include "DXGLCameraManager.h"

using namespace dxgl;

DXGLCameraManager::DXGLCameraManager() {
}

DXGLCameraManager::~DXGLCameraManager() {
}

SP_DXGLCamera DXGLCameraManager::create(DXGLWindow* window, SP_DXGLInputManager input, const std::string alias) {
	SP_DXGLCamera camera = nullptr;
	try {
		camera = std::make_shared<DXGLCamera>(window, input);
		m_cameras[alias] = camera;
	} catch (...) {
		throw std::exception("DXGLCamera could not be created.");
	}
	return camera;
}

SP_DXGLCamera DXGLCameraManager::get(const std::string alias) {
	return m_cameras[alias];
}
