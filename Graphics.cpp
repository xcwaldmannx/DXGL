#include "Graphics.h"

#pragma comment(lib, "d3d11.lib")

#include <exception>

using namespace dxgl;

Graphics::Graphics() {
    // create directx11 device and context
    D3D_DRIVER_TYPE driverTypes[] = {
    D3D_DRIVER_TYPE_HARDWARE,
    D3D_DRIVER_TYPE_WARP,
    D3D_DRIVER_TYPE_REFERENCE };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0 };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    HRESULT result = 0;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
        result = D3D11CreateDevice(NULL, driverTypes[driverTypeIndex], NULL, D3D11_CREATE_DEVICE_DEBUG,
            featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_d3dContext);
        if (SUCCEEDED(result)) {
            break;
        }
    }

    if (FAILED(result)) {
        throw std::exception("Renderer could not be created.");
    }

    m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&m_dxgiDevice);
    m_dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&m_dxgiAdapter);
    m_dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&m_dxgiFactory);
}

Graphics::~Graphics() {
    m_dxgiFactory->Release();
    m_dxgiAdapter->Release();
    m_dxgiDevice->Release();

    m_d3dContext->Release();
    m_d3dDevice->Release();
}

ID3D11Device* Graphics::device() {
    return m_d3dDevice;
}

IDXGIFactory* Graphics::factory() {
    return m_dxgiFactory;
}

ID3D11DeviceContext* Graphics::context() {
    return m_d3dContext;
}
