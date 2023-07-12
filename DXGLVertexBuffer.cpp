#include "DXGLVertexBuffer.h"

#include <d3d11.h>

using namespace dxgl;

DXGLVertexBuffer::DXGLVertexBuffer(void* vertices, int vertexCount, int vertexSize, SP_DXGLGraphics graphics) {
	m_vertexSize = vertexSize;

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = m_vertexSize * vertexCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;


	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = vertices;

	HRESULT result = graphics->device()->CreateBuffer(&bufferDesc, &subresourceData, &m_vertexBuffer);

	if (FAILED(result)) {
		throw std::exception("DXGLVertexBuffer could not be created.");
	}
}

DXGLVertexBuffer::~DXGLVertexBuffer() {
	if (m_vertexBuffer) m_vertexBuffer->Release();
}

ID3D11Buffer* DXGLVertexBuffer::getBuffer() {
	return m_vertexBuffer;
}

int DXGLVertexBuffer::getVertexSize() {
	return m_vertexSize;
}
