#pragma once

#include <string>

#include "DXGLGraphics.h"

namespace dxgl {

	class DXGLTexture2D {
	public:
		DXGLTexture2D(const std::string& filename, SP_DXGLGraphics graphics);
		DXGLTexture2D(unsigned int width, unsigned int height, unsigned char* data, SP_DXGLGraphics graphics);
		~DXGLTexture2D();

		ID3D11ShaderResourceView* get();

	private:
		ID3D11ShaderResourceView* m_srv = nullptr;
	};

}
