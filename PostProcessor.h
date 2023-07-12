#pragma once

#include "DXGLMain.h"

struct ScreenBuffer {
	float screenWidth;
	float screenHeight;
	float pad[2];
};

struct PostProcessVertex {
	Vec3f pos{};
	Vec2f texcoord{};
	Vec3f color{};
};

struct alignas(16) DrunkVisionBuffer {
	float time = 0;
	float period = 0;
	float amplitude = 0;
	float pad;
};

struct alignas(16) NightVisionBuffer {
	float time = 0;
	float amplitude = 0;
	float speed = 0;
	float pad;
};

class PostProcessor {
public:
	PostProcessor();
	~PostProcessor();

	void update(long double delta, float width, float height);
	void draw();

private:
	dxgl::SP_DXGLInputLayout m_inputLayout = nullptr;
	dxgl::SP_DXGLVertexBuffer m_vertexBuffer = nullptr;
	dxgl::SP_DXGLIndexBuffer m_indexBuffer = nullptr;
	dxgl::SP_DXGLCBuffer m_cbScreen = nullptr;
	dxgl::SP_DXGLCBuffer m_cbEffect = nullptr;

	dxgl::SP_DXGLVertexShader m_vs = nullptr;
	dxgl::SP_DXGLPixelShader m_ps = nullptr;
	dxgl::SP_DXGLPixelShader m_psDrunk = nullptr;
	dxgl::SP_DXGLPixelShader m_psNight = nullptr;
	dxgl::SP_DXGLPixelShader m_psPixel = nullptr;

	long double m_timePassed = 0;

	unsigned int m_mode = 0;
};