#include "Renderer.h"

using namespace dxgl;

Renderer::Renderer() {
	m_swapchain = std::make_shared<SwapChain>();

	m_shader = std::make_shared<DXGLShader>(Engine::graphics());
	m_raster = std::make_shared<DXGLRaster>(Engine::graphics());
	m_merger = std::make_shared<DXGLOutputMerger>(Engine::graphics());

	m_shadow = std::make_shared<DXGLShadow>();
	m_light = std::make_shared<DXGLLight>();
	m_foliageManager = std::make_shared<DXGLFoliageManager>();
	m_terrainManager = std::make_shared<DXGLTerrainManager>();
}

Renderer::~Renderer() {
}

void Renderer::init() {
	m_shadow->create();
}

void Renderer::createSwapchain(DXGLWindow* window) {
	m_swapchain->create(window);
}

void Renderer::resizeSwapchain(unsigned int width, unsigned int height) {
	m_swapchain->resize(width, height);
}

void Renderer::present(bool vsync) {
	m_swapchain->present(vsync);
}

bool Renderer::isPaused() {
	return m_swapchain->isPaused();
}

void Renderer::drawIndexedTriangleList(UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation) {
	Engine::graphics()->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::graphics()->context()->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	m_drawCallCount++;
}

void Renderer::drawIndexedTriangleListInstanced(UINT indexCount, UINT instanceCount,
	UINT startIndexLocation, UINT baseVertexLocation, UINT startInstanceLocation) {
	Engine::graphics()->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::graphics()->context()->DrawIndexedInstanced(indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	m_drawCallCount++;
}

void Renderer::drawIndexedTriangleListTess(UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation) {
	Engine::graphics()->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	Engine::graphics()->context()->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	m_drawCallCount++;
}

void Renderer::drawIndexedTriangleListInstancedTess(UINT indexCount, UINT instanceCount,
	UINT startIndexLocation, UINT baseVertexLocation, UINT startInstanceLocation) {
	Engine::graphics()->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	Engine::graphics()->context()->DrawIndexedInstanced(indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	m_drawCallCount++;
}

void Renderer::drawLineList(UINT vertexCount, UINT baseVertexLocation) {
	Engine::graphics()->context()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
	Engine::graphics()->context()->Draw(vertexCount, baseVertexLocation);
	m_drawCallCount++;
}

SP_DXGLShader Renderer::shader() {
	return m_shader;
}

SP_DXGLRaster Renderer::raster() {
	return m_raster;
}

SP_DXGLOutputMerger Renderer::merger() {
	return m_merger;
}

SP_DXGLShadow Renderer::shadow() {
	return m_shadow;
}

SP_DXGLLight Renderer::light() {
	return m_light;
}

SP_DXGLFoliageManager Renderer::foliage() {
	return m_foliageManager;
}

SP_DXGLTerrainManager Renderer::terrain() {
	return m_terrainManager;
}

void Renderer::createRenderTargetView(RESOURCE_VIEW_DESC* desc, ResourceViewSlot slot, SP_DXGLRenderTargetView* rtv) {
	m_swapchain->createRenderTargetView(desc, slot, rtv);
}

void Renderer::createDepthStencilView(RESOURCE_VIEW_DESC* desc, ResourceViewSlot slot, SP_DXGLDepthStencilView* dsv) {
	return m_swapchain->createDepthStencilView(desc, slot, dsv);
}

SP_DXGLRenderTargetView Renderer::getRTV(ResourceViewSlot slot) {
	return m_swapchain->getRTV(slot);
}

SP_DXGLDepthStencilView Renderer::getDSV(ResourceViewSlot slot) {
	return m_swapchain->getDSV(slot);
}

SP_DXGLShaderResourceView Renderer::getRenderToTexture(ResourceViewSlot slot) {
	return m_swapchain->getRenderToTexture(slot);
}

void Renderer::setFullscreen(bool fullscreen) {
	m_swapchain->setFullscreen(fullscreen);
}

void Renderer::setViewport(unsigned int width, unsigned int height) {
	D3D11_VIEWPORT viewport{};
	viewport.Width = (float) width;
	viewport.Height = (float) height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	Engine::graphics()->context()->RSSetViewports(1, &viewport);
}

void Renderer::setRenderTarget(SP_DXGLRenderTargetView rtv, const float color[4], SP_DXGLDepthStencilView dsv) {
	if (rtv && rtv->get() && dsv && dsv->get()) {
		ID3D11RenderTargetView* tempRTV = rtv->get();
		ID3D11DepthStencilView* tempDSV = dsv->get();
		Engine::graphics()->context()->OMSetRenderTargets(1, &tempRTV, tempDSV);
		Engine::graphics()->context()->ClearRenderTargetView(tempRTV, color);
	} else if (dsv && dsv->get()) {
		ID3D11DepthStencilView* tempDSV = dsv->get();
		Engine::graphics()->context()->OMSetRenderTargets(0, 0, tempDSV);
	}

	if (dsv && dsv->get()) {
		ID3D11DepthStencilView* tempDSV = dsv->get();
		Engine::graphics()->context()->ClearDepthStencilView(tempDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	}
}

void Renderer::setSamplerState(UINT slot, UINT count, ID3D11SamplerState* sampler) {
	Engine::graphics()->context()->PSSetSamplers(slot, count, &sampler);
}

void Renderer::resetDrawCallCount() {
	m_drawCallCount = 0;
}

int Renderer::getDrawCallCount() {
	return m_drawCallCount;
}
