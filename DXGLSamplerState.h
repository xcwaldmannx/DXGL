#pragma once

#include "DXGLGraphics.h"

namespace dxgl {
	class DXGLSamplerState {
	public:
		DXGLSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressUVW,
			D3D11_COMPARISON_FUNC comparison, FLOAT borderColor, SP_DXGLGraphics graphics);
		~DXGLSamplerState();

		ID3D11SamplerState* get();

	private:
		ID3D11SamplerState* m_samplerState = nullptr;
	};
}