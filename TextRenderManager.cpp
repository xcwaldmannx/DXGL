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

    hr = m_DWriteFactory->CreateTextFormat(L"Gabriola", NULL,
        DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        72.0f, L"en-us", &m_textFormat);

    if (FAILED(hr)) {
        std::cerr << "Could not create TextFormat.\n";
    }

    m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

TextRenderManager::~TextRenderManager() {
}

void TextRenderManager::renderText(const wchar_t* text, Vec3f color) {
    HWND& hwnd = Engine::window()->getWindow();
    RECT rc;
    GetClientRect(hwnd, &rc);

    ID3D11Texture2D* pTexture = nullptr;
    D3D11_TEXTURE2D_DESC texDesc{};
    texDesc.Width = rc.right - rc.left;
    texDesc.Height = rc.bottom - rc.top;
    texDesc.ArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.MiscFlags = 0;
    HRESULT hr = Engine::graphics()->device()->CreateTexture2D(&texDesc, nullptr, &pTexture);

    if (FAILED(hr)) {
        throw std::runtime_error("TextRenderManager could not create texture.");
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    //if (m_srv) m_srv->Release();
    hr = Engine::graphics()->device()->CreateShaderResourceView(pTexture, &srvDesc, &m_srv);

    if (FAILED(hr)) {
        throw std::runtime_error("TextRenderManager could not create SRV.");
    }

    IDXGISurface* pSurface = nullptr;
    hr = pTexture->QueryInterface(&pSurface);

    if (FAILED(hr)) {
        throw std::runtime_error("TextRenderManager could not get surface.");
    }

    D2D1_RENDER_TARGET_PROPERTIES renderTargetProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
        96,
        96);

    hr = m_D2DFactory->CreateDxgiSurfaceRenderTarget(
        pSurface,
        &renderTargetProps,
        &m_renderTarget
    );

    if (FAILED(hr)) {
        throw std::runtime_error("TextRenderManager could not create RenderTarget.");
    }

    ID2D1SolidColorBrush* brush = nullptr;
    hr = m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(color.x, color.y, color.z), &brush);

    if (FAILED(hr)) {
        throw std::runtime_error("TextRenderManager could not create Brush.");
    }

    m_renderTarget->BeginDraw();
    D2D1::ColorF clearColor(color.x, color.y, color.z, 0);
    m_renderTarget->Clear(clearColor);

    float dpiScaleX = 1;
    float dpiScaleY = 1;

    D2D1_RECT_F layoutRect = D2D1::RectF(
        static_cast<FLOAT>(rc.left) / dpiScaleX,
        static_cast<FLOAT>(rc.top) / dpiScaleY,
        static_cast<FLOAT>(rc.right - rc.left) / dpiScaleX,
        static_cast<FLOAT>(rc.bottom - rc.top) / dpiScaleY
    );

    UINT32 textLength = (UINT32)wcslen(text);

    m_renderTarget->DrawText(
        text,         // The string to render.
        textLength,   // The string's length.
        m_textFormat, // The text format.
        layoutRect,   // The region of the window where the text will be rendered.
        brush         // The brush used to draw the text.
    );

    m_renderTarget->EndDraw();

    brush->Release();
    m_renderTarget->Release();
    pSurface->Release();
    pTexture->Release();
}

ID3D11ShaderResourceView* TextRenderManager::getSRV() {
    return m_srv;
}
