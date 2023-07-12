#pragma once

#include "DXGLDefinitions.h"

#include "DXGLGraphics.h"

namespace dxgl {

	class DXGLVertexBuffer {
	public:
		DXGLVertexBuffer(void* vertices, int vertexCount, int vertexSize, SP_DXGLGraphics graphics);
		~DXGLVertexBuffer();

		ID3D11Buffer* getBuffer();
		int getVertexSize();

	private:
		ID3D11Buffer* m_vertexBuffer = nullptr;
		int m_vertexSize = 0;

	};

}
