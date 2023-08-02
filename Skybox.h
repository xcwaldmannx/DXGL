#pragma once

#include "DXGLDefinitions.h"
#include "DXGLMain.h"

class Skybox {
public:
	Skybox();
	~Skybox();

	void update(long double delta);
	void draw();

	dxgl::SP_TextureCube getCube();

private:
	dxgl::SP_TextureCube m_desert = nullptr;
	dxgl::SP_TextureCube m_sky = nullptr;
	dxgl::SP_TextureCube m_test = nullptr;

	dxgl::SP_InputLayout m_layout = nullptr;
	dxgl::SP_DXGLVertexShader m_vs = nullptr;
	dxgl::SP_DXGLPixelShader m_ps = nullptr;
	dxgl::SP_VSConstantBuffer m_vscb = nullptr;
};