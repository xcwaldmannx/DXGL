#include "PSConstantBuffer.h"

#include "Engine.h"
#include "Graphics.h"

using namespace dxgl;

PSConstantBuffer::PSConstantBuffer(unsigned int bytes) {
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = bytes;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	HRESULT result = Engine::graphics()->device()->CreateBuffer(&bufferDesc, nullptr, &m_cbuffer);

	if (FAILED(result)) {
		throw std::exception("PSConstantBuffer could not be created.");
	}
}

PSConstantBuffer::~PSConstantBuffer() {
	if (m_cbuffer) m_cbuffer->Release();
}

void PSConstantBuffer::bind(int slot) {
	Engine::graphics()->context()->PSSetConstantBuffers(slot, 1, &m_cbuffer);
}

void PSConstantBuffer::update(void* buffer) {
	Engine::graphics()->context()->UpdateSubresource(m_cbuffer, 0, 0, buffer, 0, 0);
}

ID3D11Buffer* PSConstantBuffer::get() {
	return m_cbuffer;
}
