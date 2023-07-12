#include "DXGLCBufferManager.h"

using namespace dxgl;

DXGLCBufferManager::DXGLCBufferManager(SP_DXGLGraphics graphics) : m_graphics(graphics) {
}

DXGLCBufferManager::~DXGLCBufferManager() {
}

SP_DXGLCBuffer DXGLCBufferManager::create(UINT bytes) {
	SP_DXGLCBuffer cbuffer = nullptr;
	try {
		cbuffer = std::make_shared<DXGLCBuffer>(bytes, m_graphics);
	} catch (...) {
		throw std::exception("DXGLCBuffer could not be created.");
	}
	return cbuffer;
}
