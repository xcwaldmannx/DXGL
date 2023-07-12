#include "DXGLRenderTargetView.h"

using namespace dxgl;

DXGLRenderTargetView::DXGLRenderTargetView(ID3D11RenderTargetView* rtv) : m_rtv(rtv) {
}

DXGLRenderTargetView::~DXGLRenderTargetView() {
	if (m_rtv) m_rtv->Release();
}

ID3D11RenderTargetView* DXGLRenderTargetView::get() {
	return m_rtv;
}