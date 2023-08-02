#pragma once

#include <d3d11.h>

#include "DXGLDefinitions.h"

#include "VertexBuffer.h"
#include "InstanceBuffer.h"
#include "IndexBuffer.h"
#include "InputLayout.h"

namespace dxgl {
	class DXGLInput {
	public:
		DXGLInput(SP_DXGLGraphics graphics);
		~DXGLInput();

		void setVertexBuffer(UINT slot, const UINT bufferCount, const SP_VertexBuffer vertexBuffers[]);
		void setIndexBuffer(const SP_IndexBuffer& indexBuffer);
		void setInstanceBuffers(const UINT bufferCount, const SP_InstanceBuffer instanceBuffers[]);
		void setInputLayout(const SP_InputLayout& inputLayout);

	private:
		SP_DXGLGraphics m_graphics = nullptr;
	};
}