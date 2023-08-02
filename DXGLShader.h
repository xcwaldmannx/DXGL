#pragma once

#include <string>
#include <unordered_map>

#include "DXGLDefinitions.h"

#include "DXGLGraphics.h"
#include "DXGLVertexShader.h"
#include "DXGLHullShader.h"
#include "DXGLDomainShader.h"
#include "DXGLPixelShader.h"
#include "Material.h"
#include "DXGLSamplerState.h"

namespace dxgl {

	struct DXGLShaderSet {
		SP_DXGLVertexShader vs = nullptr;
		SP_DXGLHullShader hs = nullptr;
		SP_DXGLDomainShader ds = nullptr;
		SP_DXGLPixelShader ps = nullptr;
	};

	class DXGLShader {
	public:
		DXGLShader(SP_DXGLGraphics graphics);
		~DXGLShader();

		template<class T>
		std::shared_ptr<T> create(const std::string& filename);
		void addSet(const DXGLShaderSet& set, const std::string& alias);
		SP_DXGLShaderSet getSet(const std::string& alias);

		void VS_setShader(const SP_DXGLVertexShader& vs);
		void HS_setShader(const SP_DXGLHullShader& hs);
		void DS_setShader(const SP_DXGLDomainShader& ds);
		void PS_setShader(const SP_DXGLPixelShader& ps);

		void setShaderSet(const DXGLShaderSet& set);
		void setShaderSet(const std::string& alias);

		void VS_setResource(UINT slot, ID3D11ShaderResourceView* srv);
		void VS_setResources(UINT slot, UINT count, ID3D11ShaderResourceView** srv);
		void PS_setResource(UINT slot, ID3D11ShaderResourceView* srv);
		void PS_setResources(UINT slot, UINT count, ID3D11ShaderResourceView** srv);

		void VS_setMaterial(UINT slot, UINT count, const SP_Material& material);
		void DS_setMaterial(UINT slot, UINT count, const SP_Material& material);
		void PS_setMaterial(UINT slot, UINT count, const SP_Material& material);

		void VS_setCBuffer(UINT slot, UINT count, ID3D11Buffer* buffers);
		void HS_setCBuffer(UINT slot, UINT count, ID3D11Buffer* buffers);
		void DS_setCBuffer(UINT slot, UINT count, ID3D11Buffer* buffers);
		void PS_setCBuffer(UINT slot, UINT count, ID3D11Buffer* buffers);

		void DS_setSampler(UINT slot, UINT count, SP_DXGLSamplerState sampler);
		void PS_setSampler(UINT slot, UINT count, SP_DXGLSamplerState sampler);

	private:
		SP_DXGLGraphics m_graphics = nullptr;
		std::unordered_map<std::string, SP_DXGLShaderSet> m_shaderSets{};
	};

}