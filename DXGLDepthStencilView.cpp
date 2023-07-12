#include "DXGLDepthStencilView.h"

using namespace dxgl;

DXGLDepthStencilView::DXGLDepthStencilView(ID3D11DepthStencilView* dsv) : m_dsv(dsv) {
}

DXGLDepthStencilView::~DXGLDepthStencilView() {
	if (m_dsv) m_dsv->Release();
}

ID3D11DepthStencilView* DXGLDepthStencilView::get() {
	return m_dsv;
}