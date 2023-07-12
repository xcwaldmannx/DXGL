#pragma once

#include "DXGLDefinitions.h"

#include "DXGLGraphics.h"

namespace dxgl {

	class DXGLInstanceBuffer {
	public:
		DXGLInstanceBuffer(void* instances, int instanceCount, int instanceSize, SP_DXGLGraphics graphics);
		~DXGLInstanceBuffer();

		void update(void* buffer);

		ID3D11Buffer* getBuffer();
		int getInstanceSize();

	private:
		SP_DXGLGraphics m_graphics = nullptr;

		ID3D11Buffer* m_instanceBuffer = nullptr;
		int m_instanceSize = 0;

	};

}
