#pragma once

#include <fstream>

#include <string>
#include <vector>
#include <any>
#include <unordered_map>

#include <d3d11.h>

#include "Bindable.h"

namespace dxgl {

	enum InputFormat {
		SINT1,
		SINT2,
		SINT3,
		SINT4,
		UINT1,
		UINT2,
		UINT3,
		UINT4,
		FLOAT1,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		TYPELESS1,
		TYPELESS2,
		TYPELESS3,
		TYPELESS4,
	};

	const std::unordered_map<InputFormat, DXGI_FORMAT> INPUT_FORMAT_MAP {
			{SINT1, DXGI_FORMAT_R32_SINT},
			{SINT2, DXGI_FORMAT_R32G32_SINT},
			{SINT3, DXGI_FORMAT_R32G32B32_SINT},
			{SINT4, DXGI_FORMAT_R32G32B32A32_SINT},
			{UINT1, DXGI_FORMAT_R32_UINT},
			{UINT2, DXGI_FORMAT_R32G32_UINT},
			{UINT3, DXGI_FORMAT_R32G32B32_UINT},
			{UINT4, DXGI_FORMAT_R32G32B32A32_UINT},
			{FLOAT1, DXGI_FORMAT_R32_FLOAT},
			{FLOAT2, DXGI_FORMAT_R32G32_FLOAT},
			{FLOAT3, DXGI_FORMAT_R32G32B32_FLOAT},
			{FLOAT4, DXGI_FORMAT_R32G32B32A32_FLOAT},
			{TYPELESS1, DXGI_FORMAT_R32_TYPELESS},
			{TYPELESS2, DXGI_FORMAT_R32G32_TYPELESS},
			{TYPELESS3, DXGI_FORMAT_R32G32B32_TYPELESS},
			{TYPELESS4, DXGI_FORMAT_R32G32B32A32_TYPELESS},
	};

	struct InputLayoutElement {
		LPCSTR semantic;
		unsigned int slot;
		DXGI_FORMAT format;
		bool isInstanced;
	};

	struct InputLayoutDesc {

		void add(LPCSTR semantic, unsigned int slot, InputFormat format, bool isInstanced) {
			InputLayoutElement element {
				semantic,
				slot,
				INPUT_FORMAT_MAP.at(format),
				isInstanced,
			};
			inputs.push_back(element);
		}

		std::vector<InputLayoutElement> inputs{};
	};

	class InputLayout : public Bindable {
	public:
		InputLayout(const InputLayoutDesc& desc, const std::string& filename);
		InputLayout(const InputLayoutDesc& desc, ID3DBlob* shaderBytecode);
		~InputLayout();

		void bind(int slot = 0) override;

		ID3D11InputLayout* get();

	private:
		std::vector<D3D11_INPUT_ELEMENT_DESC> createLayout(const InputLayoutDesc& desc);

	private:
		ID3D11InputLayout* m_layout = nullptr;
	};

}