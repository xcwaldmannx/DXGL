#pragma once

#include <d3d11.h>
#include <memory>

namespace dxgl::governor {
	class DXGLGovernor;
	typedef std::shared_ptr<DXGLGovernor> SP_DXGLGovernor;
}

namespace dxgl {
	//////////
	// DXGL //
	//////////

	class DXGLGraphics;
	typedef std::shared_ptr<DXGLGraphics> SP_DXGLGraphics;

	class DXGLSwapChain;
	typedef std::shared_ptr<DXGLSwapChain> SP_DXGLSwapChain;

	class DXGLRenderer;
	typedef std::shared_ptr<DXGLRenderer> SP_DXGLRenderer;

	class DXGLResourceManager;
	typedef std::shared_ptr<DXGLResourceManager> SP_DXGLResourceManager;

	// STAGES
	class DXGLInput;
	typedef std::shared_ptr<DXGLInput> SP_DXGLInput;
	class DXGLShader;
	typedef std::shared_ptr<DXGLShader> SP_DXGLShader;
	class DXGLRaster;
	typedef std::shared_ptr<DXGLRaster> SP_DXGLRaster;
	class DXGLOutputMerger;
	typedef std::shared_ptr<DXGLOutputMerger> SP_DXGLOutputMerger;

	class DXGLWindow;
	typedef std::shared_ptr<DXGLWindow> SP_DXGLWindow;
	class DXGLInputManager;
	typedef std::shared_ptr<DXGLInputManager> SP_DXGLInputManager;

	// Managers
	class DXGLCameraManager;
	typedef std::shared_ptr<DXGLCameraManager> SP_DXGLCameraManager;
	class DXGLCamera;
	typedef std::shared_ptr<DXGLCamera> SP_DXGLCamera;

	class DXGLShadow;
	typedef std::shared_ptr<DXGLShadow> SP_DXGLShadow;

	class DXGLLight;
	typedef std::shared_ptr<DXGLLight> SP_DXGLLight;

	class DXGLMousePicker;
	typedef std::shared_ptr<DXGLMousePicker> SP_DXGLMousePicker;

	class DXGLFoliageManager;
	typedef std::shared_ptr<DXGLFoliageManager> SP_DXGLFoliageManager;

	class DXGLTerrainManager;
	typedef std::shared_ptr<DXGLTerrainManager> SP_DXGLTerrainManager;

	///////////////
	// Resources //
	///////////////

	// buffers
	class DXGLVertexBuffer;
	typedef std::shared_ptr<DXGLVertexBuffer> SP_DXGLVertexBuffer;

	class DXGLInstanceBuffer;
	typedef std::shared_ptr<DXGLInstanceBuffer> SP_DXGLInstanceBuffer;

	class DXGLIndexBuffer;
	typedef std::shared_ptr<DXGLIndexBuffer> SP_DXGLIndexBuffer;

	class DXGLCBuffer;
	typedef std::shared_ptr<DXGLCBuffer> SP_DXGLCBuffer;

	// input layouts
	class DXGLInputLayout;
	typedef std::shared_ptr<DXGLInputLayout> SP_DXGLInputLayout;

	// Shaders
	struct DXGLShaderSet;
	typedef std::shared_ptr<DXGLShaderSet> SP_DXGLShaderSet;

	class DXGLVertexShader;
	typedef std::shared_ptr<DXGLVertexShader> SP_DXGLVertexShader;

	class DXGLHullShader;
	typedef std::shared_ptr<DXGLHullShader> SP_DXGLHullShader;

	class DXGLDomainShader;
	typedef std::shared_ptr<DXGLDomainShader> SP_DXGLDomainShader;

	class DXGLPixelShader;
	typedef std::shared_ptr<DXGLPixelShader> SP_DXGLPixelShader;

	// Assets
	class DXGLTexture2D;
	typedef std::shared_ptr<DXGLTexture2D> SP_DXGLTexture2D;

	class DXGLTextureCube;
	typedef std::shared_ptr<DXGLTextureCube> SP_DXGLTextureCube;

	class DXGLMaterial;
	typedef std::shared_ptr<DXGLMaterial> SP_DXGLMaterial;

	class DXGLMesh;
	typedef std::shared_ptr<DXGLMesh> SP_DXGLMesh;

	class DXGLBasicMesh;
	typedef std::shared_ptr<DXGLBasicMesh> SP_DXGLBasicMesh;

	// states
	class DXGLSamplerState;
	typedef std::shared_ptr<DXGLSamplerState> SP_DXGLSamplerState;

	class DXGLRasterState;
	typedef std::shared_ptr<DXGLRasterState> SP_DXGLRasterState;

	class DXGLBlendState;
	typedef std::shared_ptr<DXGLBlendState> SP_DXGLBlendState;

	class DXGLDepthStencilState;
	typedef std::shared_ptr<DXGLDepthStencilState> SP_DXGLDepthStencilState;

	// views
	class DXGLRenderTargetView;
	typedef std::shared_ptr<DXGLRenderTargetView> SP_DXGLRenderTargetView;

	class DXGLDepthStencilView;
	typedef std::shared_ptr<DXGLDepthStencilView> SP_DXGLDepthStencilView;

	class DXGLShaderResourceView;
	typedef std::shared_ptr<DXGLShaderResourceView> SP_DXGLShaderResourceView;
}
