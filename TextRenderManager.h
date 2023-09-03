#pragma once

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler")

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

	struct Text {
		std::string text{};
		float size = 24.0f;
		Vec4f bounds{};
		Vec3f color{};
		std::string alias{};
	};

	class TextRenderManager {
	public:
		TextRenderManager();
		~TextRenderManager();

		void createFont(const std::string& filename, const std::string& alias);
		void renderText(const Text& text);

		void clearText();

		ID3D11ShaderResourceView* getSRV();

	private:


		IDWriteFactory* m_DWriteFactory = nullptr;
		ID2D1Factory* m_D2DFactory = nullptr;

		unsigned int m_width = 0;
		unsigned int m_height = 0;

		ID3D11Texture2D* m_texture = nullptr;
		ID3D11ShaderResourceView* m_srv = nullptr;
		IDXGISurface* m_surface = nullptr;
		ID2D1RenderTarget* m_renderTarget = nullptr;
		ID2D1SolidColorBrush* m_brush = nullptr;

		struct FontSet {
			std::string name{};
			IDWriteFontCollection* collection = nullptr;
			IDWriteTextFormat* format = nullptr;
		};

		std::unordered_map<std::string, FontSet> m_fontSets{};

	};
}