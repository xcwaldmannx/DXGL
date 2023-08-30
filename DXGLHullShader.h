#pragma once

#include <d3d11.h>

#include "DXGLDefinitions.h"

#include "Graphics.h"

namespace dxgl {

	class DXGLHullShader {
	public:
		DXGLHullShader(void* byteCode, size_t length, SP_Graphics graphics);
		~DXGLHullShader();

		ID3D11HullShader* get();

	public:
		void* byteCode();
		size_t length();

	private:
		ID3D11HullShader* m_hullShader = nullptr;

		void* m_byteCode = nullptr;
		size_t m_length = 0;
	};

}
