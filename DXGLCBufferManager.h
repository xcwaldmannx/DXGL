#pragma once

#include "DXGLGraphics.h"
#include "DXGLCBuffer.h"

namespace dxgl {
	class DXGLCBufferManager {
	public:
		DXGLCBufferManager(SP_DXGLGraphics graphics);
		~DXGLCBufferManager();

		SP_DXGLCBuffer create(UINT bytes);

	private:
		SP_DXGLGraphics m_graphics = nullptr;
	};
}
