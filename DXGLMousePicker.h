#pragma once

#include "DXGLRenderer.h"

namespace dxgl {
	class DXGLMousePicker {
	public:
		DXGLMousePicker();
		~DXGLMousePicker();

		void create();
		void update();
		void draw();

		uint32_t getColorId(Point2f position);

	private:
		SP_DXGLRenderTargetView m_rtv = nullptr;
		SP_DXGLDepthStencilView m_dsv = nullptr;

		SP_DXGLInputLayout m_layout = nullptr;
		SP_DXGLVertexShader m_vs = nullptr;
		SP_DXGLPixelShader m_ps = nullptr;
		SP_DXGLCBuffer m_cbTrans = nullptr;

		governor::DXGLGroup* m_groupEntity{};
		governor::DXGLGroup* m_groupPickable{};

		governor::DXGLGroup m_visibleEntities{};
	};
}