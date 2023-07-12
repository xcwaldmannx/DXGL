#pragma once

#include <d3d11.h>

#include "DXGLDefinitions.h"

#include "DXGLVertexBuffer.h"
#include "DXGLInstanceBuffer.h"
#include "DXGLIndexBuffer.h"
#include "DXGLInputLayout.h"

namespace dxgl {
	class DXGLInput {
	public:
		DXGLInput(SP_DXGLGraphics graphics);
		~DXGLInput();

		void setVertexBuffer(UINT slot, const UINT bufferCount, const SP_DXGLVertexBuffer vertexBuffers[]);
		void setIndexBuffer(const SP_DXGLIndexBuffer& indexBuffer);
		void setInstanceBuffers(const UINT bufferCount, const SP_DXGLInstanceBuffer instanceBuffers[]);
		void setInputLayout(const SP_DXGLInputLayout& inputLayout);

	private:
		SP_DXGLGraphics m_graphics = nullptr;
	};
}