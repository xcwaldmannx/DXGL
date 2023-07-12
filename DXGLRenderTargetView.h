#pragma once

#include "DXGLGraphics.h"

namespace dxgl {
	class DXGLRenderTargetView {
	public:
		DXGLRenderTargetView(ID3D11RenderTargetView* rtv);
		~DXGLRenderTargetView();

		void update();

		ID3D11RenderTargetView* get();

	private:
		ID3D11RenderTargetView* m_rtv = nullptr;
	};
}