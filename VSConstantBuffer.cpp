#include "VSConstantBuffer.h"

#include "Engine.h"
#include "Graphics.h"

using namespace dxgl;

VSConstantBuffer::VSConstantBuffer(unsigned int bytes) {
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = bytes;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	HRESULT result = Engine::graphics()->device()->CreateBuffer(&bufferDesc, nullptr, &m_cbuffer);

	if (FAILED(result)) {
		throw std::exception("VSConstantBuffer could not be created.");
	}
}

VSConstantBuffer::~VSConstantBuffer() {
	if (m_cbuffer) m_cbuffer->Release();
}

void VSConstantBuffer::bind(int slot) {
	Engine::graphics()->context()->VSSetConstantBuffers(slot, 1, &m_cbuffer);
}

void VSConstantBuffer::update(void* buffer) {
	Engine::graphics()->context()->UpdateSubresource(m_cbuffer, 0, 0, buffer, 0, 0);
}

ID3D11Buffer* VSConstantBuffer::get() {
	return m_cbuffer;
}
