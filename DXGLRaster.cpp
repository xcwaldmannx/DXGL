#include "DXGLRaster.h"

using namespace dxgl;

DXGLRaster::DXGLRaster(SP_DXGLGraphics graphics) : m_graphics(graphics) {

}

DXGLRaster::~DXGLRaster() {

}

void DXGLRaster::create(const D3D11_RASTERIZER_DESC& desc, std::string alias) {
	ID3D11RasterizerState* rasterizerState;
	m_graphics->device()->CreateRasterizerState(&desc, &rasterizerState);
	m_states[alias] = rasterizerState;
}

ID3D11RasterizerState* DXGLRaster::get(std::string alias) {
	return m_states[alias];
}

void DXGLRaster::RS_setState(ID3D11RasterizerState* state) {
	m_graphics->context()->RSSetState(state);
}

void DXGLRaster::RS_setState(std::string alias) {
	m_graphics->context()->RSSetState(m_states[alias]);
}
