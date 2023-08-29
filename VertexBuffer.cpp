#include "VertexBuffer.h"

#include <d3d11.h>

using namespace dxgl;

VertexBuffer::VertexBuffer(void* vertices, int vertexCount, int vertexSize) {
	m_count = vertexCount;
	m_size = vertexSize;

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = m_size * m_count;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;


	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = vertices;

	HRESULT result = Engine::graphics()->device()->CreateBuffer(&bufferDesc, &subresourceData, &m_vbuffer);

	if (FAILED(result)) {
		throw std::exception("VertexBuffer could not be created.");
	}
}

VertexBuffer::~VertexBuffer() {
	if (m_vbuffer) m_vbuffer->Release();
}

void VertexBuffer::bind(int slot) {
	UINT stride = m_size;
	UINT offset = 0;
	Engine::graphics()->context()->IASetVertexBuffers(slot, 1, &m_vbuffer, &stride, &offset);
}

ID3D11Buffer* VertexBuffer::get() {
	return m_vbuffer;
}

int VertexBuffer::count() {
	return m_count;
}

int VertexBuffer::size() {
	return m_size;
}
