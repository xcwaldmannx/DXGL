#include "DXGLPixelShader.h"

#include <exception>

using namespace dxgl;

DXGLPixelShader::DXGLPixelShader(void* byteCode, size_t length, SP_DXGLGraphics graphics) : m_byteCode(byteCode), m_length(length) {
	HRESULT result = graphics->device()->CreatePixelShader(byteCode, length, nullptr, &m_pixelShader);

	if (FAILED(result)) {
		throw std::exception("DXGLVertexShader could not be created.");
	}
}

DXGLPixelShader::~DXGLPixelShader() {
	if (m_pixelShader) m_pixelShader->Release();
}

ID3D11PixelShader* DXGLPixelShader::get() {
	return m_pixelShader;
}

void* DXGLPixelShader::byteCode() {
	return m_byteCode;
}

size_t DXGLPixelShader::length() {
	return m_length;
}
