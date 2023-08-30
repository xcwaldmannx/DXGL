#include "IndexBuffer.h"

#include "Engine.h"
#include "Graphics.h"

using namespace dxgl;

IndexBuffer::IndexBuffer(void* indices, int indexCount) {
	m_count = indexCount;

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = 4 * indexCount;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = indices;

	HRESULT result = Engine::graphics()->device()->CreateBuffer(&bufferDesc, &initData, &m_ibuffer);

	if (FAILED(result)) {
		throw std::exception("DXGLIndexBuffer could not be created.");
	}
}

IndexBuffer::~IndexBuffer() {
	if (m_ibuffer) m_ibuffer->Release();
}

void IndexBuffer::bind(int slot) {
	Engine::graphics()->context()->IASetIndexBuffer(m_ibuffer, DXGI_FORMAT_R32_UINT, 0);
}

ID3D11Buffer* IndexBuffer::get() {
	return m_ibuffer;
}

int IndexBuffer::count() {
	return m_count;
}