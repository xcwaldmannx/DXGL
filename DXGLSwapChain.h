#pragma once

#include <d3d11.h>
#include <vector>

#include "DXGLDefinitions.h"

#include "DXGLMain.h"
#include "DXGLRenderTargetView.h"
#include "DXGLDepthStencilView.h"

namespace dxgl {

	static const int RESOURCE_VIEW_COUNT_MAX = 4;

	enum RESOURCE_FORMAT {
		FLOAT16 = DXGI_FORMAT_R16G16B16A16_FLOAT,
		UNORM8  = DXGI_FORMAT_R8G8B8A8_UNORM,
		D24_S8  = DXGI_FORMAT_D24_UNORM_S8_UINT,
	};

	struct RESOURCE_VIEW_DESC {
		RESOURCE_FORMAT format;
		int sampleCount = 0;
		int sampleQuality = 0;
	};

	struct RenderTargetData {
		SP_DXGLRenderTargetView* target;
		RESOURCE_VIEW_DESC desc;
	};

	struct DepthStencilData {
		SP_DXGLDepthStencilView* target;
		RESOURCE_VIEW_DESC desc;
	};

	enum ResourceViewSlot {
		RESOURCE_VIEW_SLOT_BACK_BUFFER = 0,
		RESOURCE_VIEW_SLOT_1 = 1,
		RESOURCE_VIEW_SLOT_2 = 2,
		RESOURCE_VIEW_SLOT_3 = 3,
	};

	class DXGLSwapChain {
	public:
		DXGLSwapChain();
		~DXGLSwapChain();

		void create(DXGLWindow* app);

		bool present(bool vsync);
		bool isPaused();

		void resize(unsigned int width, unsigned int height);
		void reload();
		void setFullscreen(bool fullscreen);

		void createRenderTargetView(RESOURCE_VIEW_DESC* desc, ResourceViewSlot slot, SP_DXGLRenderTargetView* rtv);
		void createDepthStencilView(RESOURCE_VIEW_DESC* desc, ResourceViewSlot slot, SP_DXGLDepthStencilView* dsv);

		SP_DXGLRenderTargetView getRTV(ResourceViewSlot slot);
		SP_DXGLDepthStencilView getDSV(ResourceViewSlot slot);

		SP_DXGLShaderResourceView getRenderToTexture(ResourceViewSlot slot);

	private:
		ID3D11Texture2D* createBuffer(RESOURCE_VIEW_DESC* desc, bool rtv);

	private:
		IDXGISwapChain* m_swapchain = nullptr;
		DXGLWindow* m_window = nullptr;

		unsigned int m_width = 0;
		unsigned int m_height = 0;
		bool m_isPaused = false;

		std::vector<RenderTargetData> m_rtvs{};
		std::vector<unsigned int> m_rtvSlots{};
		std::vector<DepthStencilData> m_dsvs{};
		std::vector<unsigned int> m_dsvSlots{};

	};
}
