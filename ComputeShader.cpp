/*
#include "ComputeShader.h"

#include <iostream>
#include <exception>

ComputeShader::ComputeShader(const void* byteCode, size_t length, Renderer* renderer) : m_byteCode(byteCode), m_length(length), m_renderer(renderer) {
	HRESULT result = renderer->m_d3dDevice->CreateComputeShader(m_byteCode, m_length, nullptr, &m_computeShader);

	if (FAILED(result)) {
		throw std::exception("Compute Shader could not be created.");
	}
}

ComputeShader::~ComputeShader() {
	m_computeShader->Release();
	if (m_uav) m_uav->Release();
	if (m_input) m_input->Release();
	if (m_output) m_output->Release();
}

__declspec(align(16))
struct Data {
	int x;
	int y;
};

void ComputeShader::dispatch(unsigned int x, unsigned int y, unsigned int z) {
	// bind
	m_renderer->m_d3dDeviceContext->CSSetShader(m_computeShader, nullptr, 0);
	m_renderer->m_d3dDeviceContext->CSSetShaderResources(0, 1, &m_input);
	m_renderer->m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, &m_uav, nullptr);
	Data dim{};
	ConstantBufferPtr buf = m_renderer->createConstBuffer(&dim, sizeof(Data));
	Data dim2 = {1004, 725};
	buf->update(m_renderer->context(), &dim2);
	m_renderer->context()->CS_SetCBuffers(0, 1, { &buf });

	// execute
	m_renderer->m_d3dDeviceContext->Dispatch(x, y, z);

	// unbind
	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	m_renderer->m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
}

void ComputeShader::setInputTexture(Texture2DPtr texture) {
	if (m_uav) m_uav->Release();
	if (m_output) m_output->Release();

	m_input = texture->getSRV();

	ID3D11Texture2D* outputTexture = nullptr;
	D3D11_TEXTURE2D_DESC textureDesc = texture->getDesc();
	textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	HRESULT result = m_renderer->m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, &outputTexture);

	if (FAILED(result)) {
		throw std::exception("CS output texture could not be created.");
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	result = m_renderer->m_d3dDevice->CreateUnorderedAccessView(outputTexture, &uavDesc, &m_uav);

	if (FAILED(result)) {
		throw std::exception("CS UAV could not be created.");
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	result = m_renderer->m_d3dDevice->CreateShaderResourceView(outputTexture, &srvDesc, &m_output);

	if (FAILED(result)) {
		throw std::exception("CS SRV could not be created.");
	}

	outputTexture->Release();
}

void ComputeShader::setInputSRV(ID3D11ShaderResourceView* srv, UINT width, UINT height) {
	if (m_uav) m_uav->Release();
	if (m_output) m_output->Release();

	m_input = srv;

	ID3D11Texture2D* outputTexture = nullptr;
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	HRESULT result = m_renderer->m_d3dDevice->CreateTexture2D(&textureDesc, nullptr, &outputTexture);

	if (FAILED(result)) {
		throw std::exception("CS output texture could not be created.");
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	result = m_renderer->m_d3dDevice->CreateUnorderedAccessView(outputTexture, &uavDesc, &m_uav);

	if (FAILED(result)) {
		throw std::exception("CS UAV could not be created.");
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	result = m_renderer->m_d3dDevice->CreateShaderResourceView(outputTexture, &srvDesc, &m_output);

	if (FAILED(result)) {
		throw std::exception("CS SRV could not be created.");
	}

	m_renderer->m_d3dDeviceContext->GenerateMips(m_output);

	outputTexture->Release();
}

ID3D11ShaderResourceView* ComputeShader::getSRV() {
	return m_output;
}

const void* ComputeShader::byteCode() {
	return m_byteCode;
}

size_t ComputeShader::length() {
	return m_length;
}
*/