#include "DXGLHullShader.h"

#include <exception>

using namespace dxgl;

DXGLHullShader::DXGLHullShader(void* byteCode, size_t length, SP_Graphics graphics) : m_byteCode(byteCode), m_length(length) {
	HRESULT result = graphics->device()->CreateHullShader(byteCode, length, nullptr, &m_hullShader);

	if (FAILED(result)) {
		throw std::exception("DXGLHullShader could not be created.");
	}
}

DXGLHullShader::~DXGLHullShader() {
	if (m_hullShader) m_hullShader->Release();
}

ID3D11HullShader* DXGLHullShader::get() {
	return m_hullShader;
}

void* DXGLHullShader::byteCode() {
	return m_byteCode;
}

size_t DXGLHullShader::length() {
	return m_length;
}
