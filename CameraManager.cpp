#include "CameraManager.h"

using namespace dxgl;

CameraManager::CameraManager() {
}

CameraManager::~CameraManager() {
}

SP_Camera CameraManager::create(const std::string& alias) {
	SP_Camera camera = nullptr;
	try {
		camera = std::make_shared<Camera>();
		m_cameras[alias] = camera;
	} catch (...) {
		throw std::exception("DXGLCamera could not be created.");
	}
	return camera;
}

SP_Camera CameraManager::get(const std::string& alias) {
	return m_cameras[alias];
}
