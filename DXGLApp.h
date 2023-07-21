#pragma once

#include "DXGLMain.h"

#include "DXGLComponentTypes.h"

#include "DXGLTimer.h"

#include "PostProcessor.h"
#include "Skybox.h"

#include "DXGLBasicMesh.h"

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

	// FBX Test
	SP_DXGLBasicMesh m_fbxMesh = nullptr;
	SP_DXGLBasicMesh m_fbxMan = nullptr;

	governor::DXGLGroup* m_groupEntity{};
	governor::DXGLGroup* m_groupPickable{};

	governor::DXGLGroup* m_groupFoliage{};

	governor::DXGLGroup m_visibleEntities{};

	std::vector<std::string> m_shapes{};

	governor::EntityId m_gun = 0;
	governor::EntityId m_guitar = 0;

	SP_DXGLRenderTargetView m_backBufferRTV = nullptr;
	SP_DXGLDepthStencilView m_backBufferDSV = nullptr;


	SP_DXGLCBuffer m_cbEntityBuffer = nullptr;
	SP_DXGLCBuffer m_cbSkyboxBuffer = nullptr;

	SP_DXGLCamera m_camera = nullptr;

	DXGLTimer shadowTimer;
};
