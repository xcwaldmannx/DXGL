#pragma once

#include <string>
#include <unordered_map>

#include "Graphics.h"

namespace dxgl {
	class DXGLOutputMerger {
	public:
		DXGLOutputMerger(SP_Graphics graphics);
		~DXGLOutputMerger();

		void createDepthStencil(const D3D11_DEPTH_STENCIL_DESC& desc, const std::string alias);
		void setDepthStencil(const std::string alias);

	private:
		SP_Graphics m_graphics = nullptr;
		std::unordered_map<std::string, ID3D11DepthStencilState*> m_depthStencils{};
	};
};