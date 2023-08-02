#include "HSConstantBuffer.h"

using namespace dxgl;

HSConstantBuffer::HSConstantBuffer(UINT bytes) {
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = bytes;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	HRESULT result = DXGLMain::graphics()->device()->CreateBuffer(&bufferDesc, nullptr, &m_cbuffer);

	if (FAILED(result)) {
		throw std::exception("HSConstantBuffer could not be created.");
	}
}

HSConstantBuffer::~HSConstantBuffer() {
	if (m_cbuffer) m_cbuffer->Release();
}

void HSConstantBuffer::bind(int slot) {
	DXGLMain::graphics()->context()->HSSetConstantBuffers(slot, 1, &m_cbuffer);
}

void HSConstantBuffer::update(void* buffer) {
	DXGLMain::graphics()->context()->UpdateSubresource(m_cbuffer, 0, 0, buffer, 0, 0);
}

ID3D11Buffer* HSConstantBuffer::get() {
	return m_cbuffer;
}
