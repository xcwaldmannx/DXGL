#include "DXGLCBuffer.h"

using namespace dxgl;

DXGLCBuffer::DXGLCBuffer(UINT bytes, SP_DXGLGraphics graphics) : m_graphics(graphics) {
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = bytes;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	HRESULT result = m_graphics->device()->CreateBuffer(&bufferDesc, nullptr, &m_cBuffer);

	if (FAILED(result)) {
		throw std::exception("DXGLCBuffer could not be created.");
	}
}

DXGLCBuffer::~DXGLCBuffer() {
	if (m_cBuffer) m_cBuffer->Release();
}

void DXGLCBuffer::update(void* buffer) {
	m_graphics->context()->UpdateSubresource(m_cBuffer, 0, 0, buffer, 0, 0);
}

ID3D11Buffer* DXGLCBuffer::get() {
	return m_cBuffer;
}
