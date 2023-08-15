#pragma once

#include "DXGLMain.h"

#include "DXGLComponentTypes.h"

#include "DXGLTimer.h"

#include "PostProcessor.h"
#include "Skybox.h"

#include "Mesh.h"

#include "RenderQueue.h"

struct DXGLVertex {
	Vec3f pos{};
	Vec2f texcoord{};
	Vec3f color{};
};

class DXGLApp : public dxgl::DXGLMain {
public:
	DXGLApp();
	~DXGLApp();

	void create() override;
	void update(long double delta) override;
	void draw() override;

private:
	float m_timePassed = 0.0f;
	float m_timeDelta = 0.0f;
	bool m_playState = false;
	bool m_fullscreen = false;

	PostProcessor m_postProcessor{};
	Skybox m_skybox{};

	// RenderQueue test
	RenderQueue m_queue;

	// FBX Test
	SP_Mesh m_fbxMesh = nullptr;
	SP_Mesh m_fbxMan = nullptr;

	std::vector<std::string> m_shapes{};

	governor::EntityId m_player = -1;
	governor::EntityId m_gun = -1;
	governor::EntityId m_guitar = -1;

	SP_DXGLRenderTargetView m_backBufferRTV = nullptr;
	SP_DXGLDepthStencilView m_backBufferDSV = nullptr;


	SP_VSConstantBuffer m_vscbEntityBuffer = nullptr;
	SP_PSConstantBuffer m_pscbEntityBuffer = nullptr;

	SP_VSConstantBuffer m_vscbSkyboxBuffer = nullptr;
	SP_PSConstantBuffer m_pscbSkyboxBuffer = nullptr;

	SP_DXGLCamera m_camera = nullptr;

	DXGLTimer shadowTimer;
};
