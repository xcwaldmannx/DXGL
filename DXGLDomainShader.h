#pragma once

#include <d3d11.h>

#include "DXGLDefinitions.h"

#include "DXGLGraphics.h"

namespace dxgl {

	class DXGLDomainShader {
	public:
		DXGLDomainShader(void* byteCode, size_t length, SP_DXGLGraphics graphics);
		~DXGLDomainShader();

		ID3D11DomainShader* get();

	public:
		void* byteCode();
		size_t length();

	private:
		ID3D11DomainShader* m_domainShader = nullptr;

		void* m_byteCode = nullptr;
		size_t m_length = 0;
	};

}
