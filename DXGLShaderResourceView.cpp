#include "DXGLShaderResourceView.h"

using namespace dxgl;

DXGLShaderResourceView::DXGLShaderResourceView(ID3D11ShaderResourceView* srv) : m_srv(srv) {
}

DXGLShaderResourceView::~DXGLShaderResourceView() {
	if (m_srv) m_srv->Release();
}

ID3D11ShaderResourceView* DXGLShaderResourceView::get() {
	return m_srv;
}