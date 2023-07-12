#pragma once

#include <string>

#include "DXGLGraphics.h"

namespace dxgl {
	class DXGLTextureCube {
	public:
		DXGLTextureCube(const std::string& filepath, SP_DXGLGraphics graphics);
		~DXGLTextureCube();

		ID3D11ShaderResourceView* get();

	private:
		ID3D11ShaderResourceView* m_srv = nullptr;
	};
};