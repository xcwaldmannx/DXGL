#pragma once

#include <d3d11.h>
#include <memory>

namespace dxgl {
	//////////
	// DXGL //
	//////////

	class Engine;
	typedef std::shared_ptr<Engine> SP_Engine;

	class Graphics;
	typedef std::shared_ptr<Graphics> SP_Graphics;

	class SwapChain;
	typedef std::shared_ptr<SwapChain> SP_SwapChain;

	class Renderer;
	typedef std::shared_ptr<Renderer> SP_Renderer;

	class ResourceManager;
	typedef std::shared_ptr<ResourceManager> SP_ResourceManager;

	class MousePickManager;
	typedef std::shared_ptr<MousePickManager> SP_MousePickManager;

	class EntityManager;
	typedef std::shared_ptr<EntityManager> SP_EntityManager;

	class PhysicsManager;
	typedef std::shared_ptr<PhysicsManager> SP_PhysicsManager;

	class ControllerManager;
	typedef std::shared_ptr<ControllerManager> SP_ControllerManager;

	class CameraManager;
	typedef std::shared_ptr<CameraManager> SP_CameraManager;

	// STAGES
	class DXGLShader;
	typedef std::shared_ptr<DXGLShader> SP_DXGLShader;
	class DXGLRaster;
	typedef std::shared_ptr<DXGLRaster> SP_DXGLRaster;
	class DXGLOutputMerger;
	typedef std::shared_ptr<DXGLOutputMerger> SP_DXGLOutputMerger;

	class DXGLWindow;
	typedef std::shared_ptr<DXGLWindow> SP_DXGLWindow;
	class InputManager;
	typedef std::shared_ptr<InputManager> SP_InputManager;

	// Managers
	class DXGLShadow;
	typedef std::shared_ptr<DXGLShadow> SP_DXGLShadow;

	class DXGLLight;
	typedef std::shared_ptr<DXGLLight> SP_DXGLLight;

	class DXGLFoliageManager;
	typedef std::shared_ptr<DXGLFoliageManager> SP_DXGLFoliageManager;

	class DXGLTerrainManager;
	typedef std::shared_ptr<DXGLTerrainManager> SP_DXGLTerrainManager;

	///////////////
	// Resources //
	///////////////

	// buffers
	class VertexBuffer;
	typedef std::shared_ptr<VertexBuffer> SP_VertexBuffer;

	class InstanceBuffer;
	typedef std::shared_ptr<InstanceBuffer> SP_InstanceBuffer;

	class IndexBuffer;
	typedef std::shared_ptr<IndexBuffer> SP_IndexBuffer;

	class VSConstantBuffer;
	typedef std::shared_ptr<VSConstantBuffer> SP_VSConstantBuffer;

	class HSConstantBuffer;
	typedef std::shared_ptr<HSConstantBuffer> SP_HSConstantBuffer;

	class DSConstantBuffer;
	typedef std::shared_ptr<DSConstantBuffer> SP_DSConstantBuffer;

	class PSConstantBuffer;
	typedef std::shared_ptr<PSConstantBuffer> SP_PSConstantBuffer;

	// input layouts
	class InputLayout;
	typedef std::shared_ptr<InputLayout> SP_InputLayout;

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
	class Texture2D;
	typedef std::shared_ptr<Texture2D> SP_Texture2D;

	class TextureCube;
	typedef std::shared_ptr<TextureCube> SP_TextureCube;

	class Material;
	typedef std::shared_ptr<Material> SP_Material;

	class Mesh;
	typedef std::shared_ptr<Mesh> SP_Mesh;

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
