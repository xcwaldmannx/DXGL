#pragma once

#include <unordered_map>

#include "Graphics.h"

namespace dxgl {

	class DXGLRaster {
	public:
		DXGLRaster(SP_Graphics graphics);
		~DXGLRaster();

		void create(const D3D11_RASTERIZER_DESC& desc, std::string alias);
		ID3D11RasterizerState* get(std::string alias);

		void RS_setState(ID3D11RasterizerState* state);
		void RS_setState(std::string alias);

	private:
		SP_Graphics m_graphics = nullptr;

		std::unordered_map<std::string, ID3D11RasterizerState*> m_states{};
	};
};