#include "DXGLInstanceBuffer.h"

#include <d3d11.h>

using namespace dxgl;

DXGLInstanceBuffer::DXGLInstanceBuffer(void* instances, int instanceCount, int instanceSize, SP_DXGLGraphics graphics) : m_graphics(graphics) {
	m_instanceSize = instanceSize;

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = m_instanceSize * instanceCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;


	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = instances;

	HRESULT result = m_graphics->device()->CreateBuffer(&bufferDesc, &subresourceData, &m_instanceBuffer);

	if (FAILED(result)) {
		throw std::exception("DXGLInstanceBuffer could not be created.");
	}
}

DXGLInstanceBuffer::~DXGLInstanceBuffer() {
	if (m_instanceBuffer) m_instanceBuffer->Release();
}

void DXGLInstanceBuffer::update(void* buffer) {
	m_graphics->context()->UpdateSubresource(m_instanceBuffer, 0, 0, buffer, 0, 0);
}

ID3D11Buffer* DXGLInstanceBuffer::getBuffer() {
	return m_instanceBuffer;
}

int DXGLInstanceBuffer::getInstanceSize() {
	return m_instanceSize;
}
