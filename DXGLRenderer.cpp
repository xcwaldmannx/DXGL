#include "DXGLRenderer.h"

using namespace dxgl;

DXGLRenderer::DXGLRenderer() {
	m_swapchain = std::make_shared<DXGLSwapChain>();

	m_shader = std::make_shared<DXGLShader>(DXGLMain::graphics());
	m_raster = std::make_shared<DXGLRaster>(DXGLMain::graphics());
	m_merger = std::make_shared<DXGLOutputMerger>(DXGLMain::graphics());

	m_cameraManager = std::make_shared<DXGLCameraManager>();
	m_shadow = std::make_shared<DXGLShadow>();
	m_light = std::make_shared<DXGLLight>();
	m_mousePicker = std::make_shared<DXGLMousePicker>();
	m_foliageManager = std::make_shared<DXGLFoliageManager>();
	m_terrainManager = std::make_shared<DXGLTerrainManager>();
}

DXGLRenderer::~DXGLRenderer() {
}

void DXGLRenderer::init() {
	m_shadow->create();
	m_mousePicker->create();
}

void DXGLRenderer::createSwapchain(DXGLWindow* window) {
	m_swapchain->create(window);
}

void DXGLRenderer::resizeSwapchain(unsigned int width, unsigned int height) {
	m_swapchain->resize(width, height);
}

void DXGLRenderer::present(bool vsync) {
	m_swapchain->present(vsync);
}

bool DXGLRenderer::isPaused() {
	return m_swapchain->isPaused();
}

void DXGLRenderer::drawIndexedTriangleList(UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation) {
	DXGLMain::graphics()->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DXGLMain::graphics()->context()->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	m_drawCallCount++;
}

void DXGLRenderer::drawIndexedTriangleListInstanced(UINT indexCount, UINT instanceCount,
	UINT startIndexLocation, UINT baseVertexLocation, UINT startInstanceLocation) {
	DXGLMain::graphics()->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DXGLMain::graphics()->context()->DrawIndexedInstanced(indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	m_drawCallCount++;
}

void DXGLRenderer::drawIndexedTriangleListTess(UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation) {
	DXGLMain::graphics()->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	DXGLMain::graphics()->context()->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	m_drawCallCount++;
}

void DXGLRenderer::drawIndexedTriangleListInstancedTess(UINT indexCount, UINT instanceCount,
	UINT startIndexLocation, UINT baseVertexLocation, UINT startInstanceLocation) {
	DXGLMain::graphics()->context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	DXGLMain::graphics()->context()->DrawIndexedInstanced(indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
	m_drawCallCount++;
}

void DXGLRenderer::drawLineList(UINT vertexCount, UINT baseVertexLocation) {
	DXGLMain::graphics()->context()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
	DXGLMain::graphics()->context()->Draw(vertexCount, baseVertexLocation);
	m_drawCallCount++;
}

SP_DXGLShader DXGLRenderer::shader() {
	return m_shader;
}

SP_DXGLRaster DXGLRenderer::raster() {
	return m_raster;
}

SP_DXGLOutputMerger DXGLRenderer::merger() {
	return m_merger;
}

SP_DXGLCameraManager DXGLRenderer::camera() {
	return m_cameraManager;
}

SP_DXGLShadow DXGLRenderer::shadow() {
	return m_shadow;
}

SP_DXGLLight DXGLRenderer::light() {
	return m_light;
}

SP_DXGLMousePicker DXGLRenderer::mousePicker() {
	return m_mousePicker;
}

SP_DXGLFoliageManager DXGLRenderer::foliage() {
	return m_foliageManager;
}

SP_DXGLTerrainManager DXGLRenderer::terrain() {
	return m_terrainManager;
}

void DXGLRenderer::createRenderTargetView(RESOURCE_VIEW_DESC* desc, ResourceViewSlot slot, SP_DXGLRenderTargetView* rtv) {
	m_swapchain->createRenderTargetView(desc, slot, rtv);
}

void DXGLRenderer::createDepthStencilView(RESOURCE_VIEW_DESC* desc, ResourceViewSlot slot, SP_DXGLDepthStencilView* dsv) {
	return m_swapchain->createDepthStencilView(desc, slot, dsv);
}

SP_DXGLRenderTargetView DXGLRenderer::getRTV(ResourceViewSlot slot) {
	return m_swapchain->getRTV(slot);
}

SP_DXGLDepthStencilView DXGLRenderer::getDSV(ResourceViewSlot slot) {
	return m_swapchain->getDSV(slot);
}

SP_DXGLShaderResourceView DXGLRenderer::getRenderToTexture(ResourceViewSlot slot) {
	return m_swapchain->getRenderToTexture(slot);
}

void DXGLRenderer::setFullscreen(bool fullscreen) {
	m_swapchain->setFullscreen(fullscreen);
}

void DXGLRenderer::setViewport(unsigned int width, unsigned int height) {
	D3D11_VIEWPORT viewport{};
	viewport.Width = (float) width;
	viewport.Height = (float) height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	DXGLMain::graphics()->context()->RSSetViewports(1, &viewport);
}

void DXGLRenderer::setRenderTarget(SP_DXGLRenderTargetView rtv, const float color[4], SP_DXGLDepthStencilView dsv) {
	if (rtv && rtv->get() && dsv && dsv->get()) {
		ID3D11RenderTargetView* tempRTV = rtv->get();
		ID3D11DepthStencilView* tempDSV = dsv->get();
		DXGLMain::graphics()->context()->OMSetRenderTargets(1, &tempRTV, tempDSV);
		DXGLMain::graphics()->context()->ClearRenderTargetView(tempRTV, color);
	} else if (dsv && dsv->get()) {
		ID3D11DepthStencilView* tempDSV = dsv->get();
		DXGLMain::graphics()->context()->OMSetRenderTargets(0, 0, tempDSV);
	}

	if (dsv && dsv->get()) {
		ID3D11DepthStencilView* tempDSV = dsv->get();
		DXGLMain::graphics()->context()->ClearDepthStencilView(tempDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	}
}

void DXGLRenderer::setSamplerState(UINT slot, UINT count, ID3D11SamplerState* sampler) {
	DXGLMain::graphics()->context()->PSSetSamplers(slot, count, &sampler);
}

void DXGLRenderer::resetDrawCallCount() {
	m_drawCallCount = 0;
}

int DXGLRenderer::getDrawCallCount() {
	return m_drawCallCount;
}
