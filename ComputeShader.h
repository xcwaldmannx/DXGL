/*
#pragma once

#include <d3d11.h>

class ComputeShader {
public:
	ComputeShader(const void* byteCode, size_t length, Renderer* renderer);
	~ComputeShader();

	void dispatch(unsigned int x, unsigned int y, unsigned int z);

	void setInputTexture(Texture2DPtr texture);
	void setInputSRV(ID3D11ShaderResourceView* srv, UINT width, UINT height);

	ID3D11ShaderResourceView* getSRV();

public:
	const void* byteCode();
	size_t length();

private:
	Renderer* m_renderer = nullptr;
	ID3D11ComputeShader* m_computeShader = nullptr;

	ID3D11UnorderedAccessView* m_uav = nullptr;
	ID3D11ShaderResourceView* m_input = nullptr;
	ID3D11ShaderResourceView* m_output = nullptr;

	const void* m_byteCode = nullptr;
	size_t m_length;

	friend class DeviceContext;
};
*/
