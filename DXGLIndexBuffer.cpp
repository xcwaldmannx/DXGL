#include "DXGLIndexBuffer.h"

using namespace dxgl;

DXGLIndexBuffer::DXGLIndexBuffer(void* indices, int indexCount, SP_DXGLGraphics graphics) {
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = 4 * indexCount;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = indices;

	HRESULT result = graphics->device()->CreateBuffer(&bufferDesc, &initData, &m_indexBuffer);

	if (FAILED(result)) {
		throw std::exception("DXGLIndexBuffer could not be created.");
	}
}

DXGLIndexBuffer::~DXGLIndexBuffer() {
	if (m_indexBuffer) m_indexBuffer->Release();
}

ID3D11Buffer* DXGLIndexBuffer::getBuffer() {
	return m_indexBuffer;
}
