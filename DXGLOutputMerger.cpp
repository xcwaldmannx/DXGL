#include "DXGLOutputMerger.h"

using namespace dxgl;

DXGLOutputMerger::DXGLOutputMerger(SP_DXGLGraphics graphics) : m_graphics(graphics) {

}

DXGLOutputMerger::~DXGLOutputMerger() {
	for (auto i = m_depthStencils.begin(); i != m_depthStencils.end(); i++) {
		i->second->Release();
	}
}

void DXGLOutputMerger::createDepthStencil(const D3D11_DEPTH_STENCIL_DESC& desc, const std::string alias) {
	ID3D11DepthStencilState* depthStencilState = nullptr;
	try {
		m_graphics->device()->CreateDepthStencilState(&desc, &depthStencilState);
		m_depthStencils[alias] = depthStencilState;
	} catch (...) {
		throw std::exception("DXGLOutputMerger Depth Stencil could not be created.");
	}
}

void DXGLOutputMerger::setDepthStencil(const std::string alias) {
	m_graphics->context()->OMSetDepthStencilState(m_depthStencils[alias], 0);
}
