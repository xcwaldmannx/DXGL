#include "InstanceBuffer.h"

#include <d3d11.h>

using namespace dxgl;

InstanceBuffer::InstanceBuffer(void* instances, int instanceCount, int instanceSize) {
	m_count = instanceCount;
	m_size = instanceSize;

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = m_size * m_count;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;


	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = instances;

	HRESULT result = Engine::graphics()->device()->CreateBuffer(&bufferDesc, &subresourceData, &m_ibuffer);

	if (FAILED(result)) {
		throw std::exception("InstanceBuffer could not be created.");
	}
}

InstanceBuffer::~InstanceBuffer() {
	if (m_ibuffer) m_ibuffer->Release();
}

void InstanceBuffer::bind(int slot) {
	UINT stride = m_size;
	UINT offset = 0;
	Engine::graphics()->context()->IASetVertexBuffers(slot, 1, &m_ibuffer, &stride, &offset);
}

void InstanceBuffer::update(void* buffer) {
	Engine::graphics()->context()->UpdateSubresource(m_ibuffer, 0, 0, buffer, 0, 0);
}

ID3D11Buffer* InstanceBuffer::get() {
	return m_ibuffer;
}

int InstanceBuffer::count() {
	return m_count;
}

int InstanceBuffer::size() {
	return m_size;
}
