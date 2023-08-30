#pragma once

#include <d3d11.h>

#include "DXGLDefinitions.h"

#include "Graphics.h"

namespace dxgl {

	class DXGLPixelShader {
	public:
		DXGLPixelShader(void* byteCode, size_t length, SP_Graphics graphics);
		~DXGLPixelShader();

		ID3D11PixelShader* get();

	public:
		void* byteCode();
		size_t length();

	private:
		ID3D11PixelShader* m_pixelShader = nullptr;

		void* m_byteCode = nullptr;
		size_t m_length = 0;
	};

}
