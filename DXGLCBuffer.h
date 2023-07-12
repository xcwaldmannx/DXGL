#pragma once

#include "DXGLGraphics.h"

namespace dxgl {

	class DXGLCBuffer {
	public:
		DXGLCBuffer(UINT bytes, SP_DXGLGraphics graphics);
		~DXGLCBuffer();

		void update(void* buffer);

		ID3D11Buffer* get();

	private:
		SP_DXGLGraphics m_graphics = nullptr;
		ID3D11Buffer* m_cBuffer = nullptr;
	};

}
