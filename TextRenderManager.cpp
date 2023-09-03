#include "TextRenderManager.h"

#include <iostream>

#include "Engine.h"
#include "ResourceManager.h"

using namespace dxgl;

TextRenderManager::TextRenderManager() {
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_D2DFactory);

    if (FAILED(hr)) {
        std::cerr << "Could not create D2DFactory.\n";
    }

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_DWriteFactory));

    if (FAILED(hr)) {
        std::cerr << "Could not create DWFactory.\n";
    }
}

TextRenderManager::~TextRenderManager() {
}

void TextRenderManager::createFont(const std::string& filename, const std::string& alias) {

    FontSet set{};
    set.name = alias;

    std::wstring widestr = std::wstring(filename.begin(), filename.end());
    const wchar_t* widecstr = widestr.c_str();

    if (AddFontResource(widecstr) == 0)  {
        throw std::runtime_error("Error adding font resource!");
    }

    if (FAILED(m_DWriteFactory->GetSystemFontCollection(&set.collection, false))) {
        throw std::runtime_error("Failed to retrieve system font collection.");
    }

    if (FAILED(m_DWriteFactory->CreateTextFormat(L"heavy heap", set.collection,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 48.0f, L"en-us", &set.format))) {
        throw std::runtime_error("Failed to create DirectWrite text format.");
    }

    set.format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    set.format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    m_fontSets[alias] = set;

    std::cout << "Loaded font '" << alias << "'.\n";
}

void TextRenderManager::renderText(const Text& text) {

    FontSet& set = m_fontSets[text.alias];

    HWND& hwnd = Engine::window()->getWindow();
    RECT rc;
    GetClientRect(hwnd, &rc);

    unsigned int width = rc.right - rc.left;
    unsigned int height = rc.bottom - rc.top;

    if (m_width != width || m_height != height) {
        m_width = width;
        m_height = height;

        D3D11_TEXTURE2D_DESC texDesc{};
        texDesc.Width = m_width;
        texDesc.Height = m_height;
        texDesc.ArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.MiscFlags = 0;
        HRESULT hr = Engine::graphics()->device()->CreateTexture2D(&texDesc, nullptr, &m_texture);

        if (FAILED(hr)) {
            throw std::runtime_error("TextRenderManager could not create texture.");
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;

        hr = Engine::graphics()->device()->CreateShaderResourceView(m_texture, &srvDesc, &m_srv);

        if (FAILED(hr)) {
            throw std::runtime_error("TextRenderManager could not create SRV.");
        }

        hr = m_texture->QueryInterface(&m_surface);

        if (FAILED(hr)) {
            throw std::runtime_error("TextRenderManager could not get surface.");
        }

        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            96, 96);

        hr = m_D2DFactory->CreateDxgiSurfaceRenderTarget(
            m_surface,
            &props,
            &m_renderTarget
        );

        if (FAILED(hr)) {
            throw std::runtime_error("TextRenderManager could not create RenderTarget.");
        }

        hr = m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(text.color.x, text.color.y, text.color.z), &m_brush);

        if (FAILED(hr)) {
            throw std::runtime_error("TextRenderManager could not create Brush.");
        }
    }

    if (!m_renderTarget) return;

    m_renderTarget->BeginDraw();
    float dpiScaleX = 1;
    float dpiScaleY = 1;

    D2D1_RECT_F layoutRect = D2D1::RectF(
        static_cast<FLOAT>((rc.right - rc.left) * text.bounds.x) / dpiScaleX,
        static_cast<FLOAT>((rc.bottom - rc.top) * text.bounds.y) / dpiScaleY,
        static_cast<FLOAT>((rc.right - rc.left) * text.bounds.z) / dpiScaleX,
        static_cast<FLOAT>((rc.bottom - rc.top) * text.bounds.w) / dpiScaleY
    );

    std::wstring widestr = std::wstring(text.text.begin(), text.text.end());
    const wchar_t* widecstr = widestr.c_str();

    UINT32 textLength = (UINT32) wcslen(widecstr);

    m_renderTarget->DrawText(
        widecstr,     // The string to render.
        textLength,   // The string's length.
        set.format,   // The text format.
        layoutRect,   // The region of the window where the text will be rendered.
        m_brush       // The brush used to draw the text.
    );

    m_renderTarget->EndDraw();
}

void TextRenderManager::clearText() {
    if (!m_renderTarget) return;
    m_renderTarget->BeginDraw();
    D2D1::ColorF clearColor(0, 0, 0, 0);
    m_renderTarget->Clear(clearColor);
    m_renderTarget->EndDraw();
}

ID3D11ShaderResourceView* TextRenderManager::getSRV() {
    return m_srv;
}
