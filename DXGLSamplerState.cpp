#include "DXGLSamplerState.h"

using namespace dxgl;

DXGLSamplerState::DXGLSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressUVW,
	D3D11_COMPARISON_FUNC comparison, FLOAT borderColor, SP_DXGLGraphics graphics) {

	D3D11_SAMPLER_DESC desc{};
	desc.Filter = filter;
	desc.AddressU = addressUVW;
	desc.AddressV = addressUVW;
	desc.AddressW = addressUVW;
	desc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	desc.ComparisonFunc = comparison;
	desc.BorderColor[0] = borderColor;
	desc.BorderColor[1] = borderColor;
	desc.BorderColor[2] = borderColor;
	desc.BorderColor[3] = borderColor;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT result = graphics->device()->CreateSamplerState(&desc, &m_samplerState);

	if (FAILED(result)) {
		throw std::exception("DXGLSamplerState could not be created.");
	}
}

DXGLSamplerState::~DXGLSamplerState() {
	if (m_samplerState) m_samplerState->Release();
}

ID3D11SamplerState* DXGLSamplerState::get() {
	return m_samplerState;
}
