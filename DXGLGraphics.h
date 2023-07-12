#pragma once

#include "DXGLDefinitions.h"

namespace dxgl {

	class DXGLGraphics {
	public:
		DXGLGraphics();
		~DXGLGraphics();

		ID3D11Device* device();
		IDXGIFactory* factory();
		ID3D11DeviceContext* context();

	private:
		ID3D11Device* m_d3dDevice = nullptr;
		ID3D11DeviceContext* m_d3dContext = nullptr;

		IDXGIDevice* m_dxgiDevice = nullptr;
		IDXGIAdapter* m_dxgiAdapter = nullptr;
		IDXGIFactory* m_dxgiFactory = nullptr;

		D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;
	};
}