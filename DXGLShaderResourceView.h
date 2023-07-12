#pragma once

#include "DXGLGraphics.h"

namespace dxgl {
	class DXGLShaderResourceView {
	public:
		DXGLShaderResourceView(ID3D11ShaderResourceView* srv);
		~DXGLShaderResourceView();

		ID3D11ShaderResourceView* get();

	private:
		ID3D11ShaderResourceView* m_srv = nullptr;
	};
}