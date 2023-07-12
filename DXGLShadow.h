#pragma once

#include <d3d11.h>

#include <vector>

#include "DXGLMain.h"
#include "DXGLMesh.h"

namespace dxgl {

	struct alignas(16) TransformBuffer {
		Mat4f world{};
		Mat4f view{};
		Mat4f proj{};
	};

	struct alignas(16) CascadeBuffer {
		Mat4f view{};
		Mat4f proj{};
		float resolution = 0;
		float pcfSampleCount = 0;
	};

	struct alignas(16) ShadowBuffer {
		CascadeBuffer cascades[4];
	};

	struct ShadowCascade {
		SP_DXGLDepthStencilView dsv = nullptr;
		ID3D11ShaderResourceView* srv = nullptr;
		float scale = 0;
		CascadeBuffer buffer{};
	};


	class DXGLShadow {
	public:
		DXGLShadow();
		~DXGLShadow();

		void create();
		void update(Vec3f position, Vec3f target);
		void draw();

		ID3D11ShaderResourceView** getSRVs();

	private:
		void createCascades();

	private:
		SP_DXGLInputLayout m_layout = nullptr;
		SP_DXGLVertexShader m_vs = nullptr;
		SP_DXGLPixelShader m_ps = nullptr;
		SP_DXGLCBuffer m_cbTrans = nullptr;

		std::vector<ShadowCascade> m_cascades{};
		SP_DXGLCBuffer m_cbShadow = nullptr;

		governor::DXGLGroup* m_groupEntity{};
		governor::DXGLGroup m_visibleEntities{};

		std::unordered_map<SP_DXGLMesh, governor::DXGLGroup> m_meshGroups{};

		ID3D11ShaderResourceView** m_srvs = nullptr;
	};

}