#pragma once

#include <d3d11.h>

namespace dxgl {
	class DXGLDepthStencilView {
	public:
		DXGLDepthStencilView(ID3D11DepthStencilView* dsv);
		~DXGLDepthStencilView();

		ID3D11DepthStencilView* get();

	private:
		ID3D11DepthStencilView* m_dsv = nullptr;
	};
}