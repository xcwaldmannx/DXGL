#pragma once

#include <d3d11.h>

#include "DXGLDefinitions.h"

#include "Graphics.h"

namespace dxgl {

	class DXGLVertexShader {
	public:
		DXGLVertexShader(void* byteCode, size_t length, SP_Graphics graphics);
		~DXGLVertexShader();

		ID3D11VertexShader* get();

	public:
		void* byteCode();
		size_t length();

	private:
		ID3D11VertexShader* m_vertexShader = nullptr;

		void* m_byteCode = nullptr;
		size_t m_length = 0;
	};

}
