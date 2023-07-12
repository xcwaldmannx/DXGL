#pragma once

#include "DXGLDefinitions.h"
#include "DXGLMain.h"

class Skybox {
public:
	Skybox();
	~Skybox();

	void update(long double delta);
	void draw();

	dxgl::SP_DXGLTextureCube getCube();

private:
	dxgl::SP_DXGLTextureCube m_desert = nullptr;
	dxgl::SP_DXGLTextureCube m_sky = nullptr;
	dxgl::SP_DXGLTextureCube m_test = nullptr;

	dxgl::SP_DXGLInputLayout m_layout = nullptr;
	dxgl::SP_DXGLVertexShader m_vs = nullptr;
	dxgl::SP_DXGLPixelShader m_ps = nullptr;
	dxgl::SP_DXGLCBuffer m_cb = nullptr;
};