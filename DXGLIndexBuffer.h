#pragma once

#include "DXGLDefinitions.h"

#include "DXGLGraphics.h"

namespace dxgl {

	class DXGLIndexBuffer {
	public:
		DXGLIndexBuffer(void* indices, int indexCount, SP_DXGLGraphics graphics);
		~DXGLIndexBuffer();

		ID3D11Buffer* getBuffer();

	private:
		ID3D11Buffer* m_indexBuffer = nullptr;
	};

}