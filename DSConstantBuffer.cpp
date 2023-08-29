#include "DSConstantBuffer.h"

using namespace dxgl;

DSConstantBuffer::DSConstantBuffer(UINT bytes) {
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = bytes;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	HRESULT result = Engine::graphics()->device()->CreateBuffer(&bufferDesc, nullptr, &m_cbuffer);

	if (FAILED(result)) {
		throw std::exception("DSConstantBuffer could not be created.");
	}
}

DSConstantBuffer::~DSConstantBuffer() {
	if (m_cbuffer) m_cbuffer->Release();
}

void DSConstantBuffer::bind(int slot) {
	Engine::graphics()->context()->DSSetConstantBuffers(slot, 1, &m_cbuffer);
}

void DSConstantBuffer::update(void* buffer) {
	Engine::graphics()->context()->UpdateSubresource(m_cbuffer, 0, 0, buffer, 0, 0);
}

ID3D11Buffer* DSConstantBuffer::get() {
	return m_cbuffer;
}
