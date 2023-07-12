#pragma once

#include "DXGLDefinitions.h"

#include "DXGLGraphics.h"
#include "DXGLSwapChain.h"

#include "DXGLGovernor.h"

#include "DXGLInput.h"
#include "DXGLShader.h"
#include "DXGLRaster.h"
#include "DXGLOutputMerger.h"

#include "DXGLCBufferManager.h"

#include "DXGLCameraManager.h"

#include "DXGLShadow.h"
#include "DXGLLight.h"
#include "DXGLMousePicker.h"

namespace dxgl {

	enum RESOURCE_FORMAT;
	struct RESOURCE_VIEW_DESC;
	enum ResourceViewSlot;

	class DXGLRenderer {
	public:
		DXGLRenderer();
		~DXGLRenderer();

		void init();

		void createSwapchain(DXGLWindow* window);
		void resizeSwapchain(unsigned int width, unsigned int height);
		void present(bool vsync);
		bool isPaused();

		void drawIndexedTriangleList(UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation);
		void drawIndexedTriangleListInstanced(UINT indexCount, UINT instanceCount,
			UINT startIndexLocation, UINT baseVertexLocation, UINT startInstanceLocation);
		void drawIndexedTriangleListTess(UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation);
		void drawIndexedTriangleListInstancedTess(UINT indexCount, UINT instanceCount,
			UINT startIndexLocation, UINT baseVertexLocation, UINT startInstanceLocation);
		void drawLineList(UINT vertexCount, UINT baseVertexLocation);

		SP_DXGLInput input();
		SP_DXGLShader shader();
		SP_DXGLRaster raster();
		SP_DXGLOutputMerger merger();

		SP_DXGLCameraManager camera();

		SP_DXGLShadow shadow();
		SP_DXGLLight light();
		SP_DXGLMousePicker mousePicker();

		void createRenderTargetView(RESOURCE_VIEW_DESC* desc, ResourceViewSlot slot, SP_DXGLRenderTargetView* rtv);
		void createDepthStencilView(RESOURCE_VIEW_DESC* desc, ResourceViewSlot slot, SP_DXGLDepthStencilView* dsv);

		SP_DXGLRenderTargetView getRTV(ResourceViewSlot slot);
		SP_DXGLDepthStencilView getDSV(ResourceViewSlot slot);

		SP_DXGLShaderResourceView getRenderToTexture(ResourceViewSlot slot);

		void setFullscreen(bool fullscreen);

		void setViewport(unsigned int width, unsigned int height);
		void setRenderTarget(SP_DXGLRenderTargetView rtv, const float color[4], SP_DXGLDepthStencilView dsv);

		void setSamplerState(UINT slot, UINT count, ID3D11SamplerState* sampler);

		void resetDrawCallCount();
		int getDrawCallCount();

	private:
		SP_DXGLSwapChain m_swapchain = nullptr;

		SP_DXGLInput m_input = nullptr;
		SP_DXGLShader m_shader = nullptr;
		SP_DXGLRaster m_raster = nullptr;
		SP_DXGLOutputMerger m_merger = nullptr;

		SP_DXGLCameraManager m_cameraManager = nullptr;

		SP_DXGLShadow m_shadow = nullptr;

		SP_DXGLLight m_light = nullptr;

		SP_DXGLMousePicker m_mousePicker = nullptr;

		int m_drawCallCount = 0;
	};

}
