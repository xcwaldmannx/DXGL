#include "VSConstantBuffer.h"

using namespace dxgl;

VSConstantBuffer::VSConstantBuffer(UINT bytes) {
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = bytes;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	HRESULT result = DXGLMain::graphics()->device()->CreateBuffer(&bufferDesc, nullptr, &m_cbuffer);

	if (FAILED(result)) {
		throw std::exception("VSConstantBuffer could not be created.");
	}
}

VSConstantBuffer::~VSConstantBuffer() {
	if (m_cbuffer) m_cbuffer->Release();
}

void VSConstantBuffer::bind(int slot) {
	DXGLMain::graphics()->context()->VSSetConstantBuffers(slot, 1, &m_cbuffer);
}

void VSConstantBuffer::update(void* buffer) {
	DXGLMain::graphics()->context()->UpdateSubresource(m_cbuffer, 0, 0, buffer, 0, 0);
}

ID3D11Buffer* VSConstantBuffer::get() {
	return m_cbuffer;
}
