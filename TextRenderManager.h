#pragma once

#pragma comment(lib, "d3dcompiler")
#include <d3dcompiler.h>

#include <d2d1.h>
#pragma comment(lib, "d2d1.lib")
#include <dwrite.h>
#pragma comment(lib, "dwrite.lib")

#include <string>
#include <unordered_map>

#include "DXGLDefinitions.h"

#include "Vec2f.h"
#include "Vec3f.h"
#include "Mat4f.h"

namespace dxgl {
	class TextRenderManager {
	public:
		TextRenderManager();
		~TextRenderManager();

		void renderText(const wchar_t* text, Vec3f color);

		ID3D11ShaderResourceView* getSRV();

	private:

		ID3D11ShaderResourceView* m_srv = nullptr;

		IDWriteFactory* m_DWriteFactory = nullptr;
		IDWriteTextFormat* m_textFormat = nullptr;

		ID2D1Factory* m_D2DFactory = nullptr;
		ID2D1RenderTarget* m_renderTarget = nullptr;
		ID2D1SolidColorBrush* m_blackBrush = nullptr;

	};
}