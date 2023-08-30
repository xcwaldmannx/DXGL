#include "DXGLDomainShader.h"

#include <exception>

using namespace dxgl;

DXGLDomainShader::DXGLDomainShader(void* byteCode, size_t length, SP_Graphics graphics) : m_byteCode(byteCode), m_length(length) {
	HRESULT result = graphics->device()->CreateDomainShader(byteCode, length, nullptr, &m_domainShader);

	if (FAILED(result)) {
		throw std::exception("DXGLDomainShader could not be created.");
	}
}

DXGLDomainShader::~DXGLDomainShader() {
	if (m_domainShader) m_domainShader->Release();
}

ID3D11DomainShader* DXGLDomainShader::get() {
	return m_domainShader;
}

void* DXGLDomainShader::byteCode() {
	return m_byteCode;
}

size_t DXGLDomainShader::length() {
	return m_length;
}
