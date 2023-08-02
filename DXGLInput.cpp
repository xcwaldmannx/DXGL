#include "DXGLInput.h"

using namespace dxgl;

DXGLInput::DXGLInput(SP_DXGLGraphics graphics) : m_graphics(graphics) {
}

DXGLInput::~DXGLInput() {
}

void DXGLInput::setVertexBuffer(UINT slot, const UINT bufferCount, const SP_VertexBuffer vertexBuffers[]) {
	ID3D11Buffer** buffers = new ID3D11Buffer*[bufferCount];
	UINT* stride = new UINT[bufferCount];
	UINT offset = 0;
	for (int i = 0; i < bufferCount; i++) {
		buffers[i] = vertexBuffers[i]->get();
		stride[i] = vertexBuffers[i]->size();
	}
	m_graphics->context()->IASetVertexBuffers(slot, bufferCount, buffers, stride, &offset);
	delete[] stride;
	delete[] buffers;
}

void DXGLInput::setInstanceBuffers(const UINT bufferCount, const SP_InstanceBuffer instanceBuffers[]) {
	ID3D11Buffer** buffers = new ID3D11Buffer * [bufferCount];
	UINT* stride = new UINT[bufferCount];
	UINT offset = 0;
	for (int i = 0; i < bufferCount; i++) {
		buffers[i] = instanceBuffers[i]->get();
		stride[i] = instanceBuffers[i]->size();
	}
	m_graphics->context()->IASetVertexBuffers(1, bufferCount, buffers, stride, &offset);
	delete[] stride;
	delete[] buffers;
}

void DXGLInput::setIndexBuffer(const SP_IndexBuffer& indexBuffer) {
	m_graphics->context()->IASetIndexBuffer(indexBuffer->get(), DXGI_FORMAT_R32_UINT, 0);
}

void DXGLInput::setInputLayout(const SP_InputLayout& inputLayout) {
	m_graphics->context()->IASetInputLayout(inputLayout->get());
}
