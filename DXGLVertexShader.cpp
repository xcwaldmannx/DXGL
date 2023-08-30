#include "DXGLVertexShader.h"

#include <exception>

using namespace dxgl;

DXGLVertexShader::DXGLVertexShader(void* byteCode, size_t length, SP_Graphics graphics) : m_byteCode(byteCode), m_length(length) {
	HRESULT result = graphics->device()->CreateVertexShader(byteCode, length, nullptr, &m_vertexShader);

	if (FAILED(result)) {
		throw std::exception("DXGLVertexShader could not be created.");
	}
}

DXGLVertexShader::~DXGLVertexShader() {
	if (m_vertexShader) m_vertexShader->Release();
}

ID3D11VertexShader* DXGLVertexShader::get() {
	return m_vertexShader;
}

void* DXGLVertexShader::byteCode() {
	return m_byteCode;
}

size_t DXGLVertexShader::length() {
	return m_length;
}
