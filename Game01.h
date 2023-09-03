#pragma once

#include "Engine.h"
#include "RenderQueue.h"

class Game01 : public dxgl::Engine {
public:
	Game01();
	~Game01();

	void create() override;
	void update(long double delta) override;
	void draw() override;

private:
	dxgl::RenderQueue m_renderQueue;

	dxgl::governor::EntityId m_player;
	float m_playerSpeed = 16.0f;

	bool m_fullscreen = false;
	bool m_playstate = false;

	SP_DXGLRenderTargetView m_backbufferRTV = nullptr;
	SP_DXGLDepthStencilView m_backbufferDSV = nullptr;

};