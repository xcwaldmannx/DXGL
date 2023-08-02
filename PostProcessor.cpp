#include "PostProcessor.h"

PostProcessor::PostProcessor() {
	dxgl::InputLayoutDesc descPost{};
	descPost.add("POSITION", 0, dxgl::FLOAT3, false);
	descPost.add("TEXCOORD", 0, dxgl::FLOAT2, false);
	m_inputLayout = dxgl::DXGLMain::resource()->createInputLayout(descPost, "Assets/Shaders/VS_PostProcess.cso");

	std::vector<PostProcessVertex> vertices = {
	{ {-1.0f, 1.0f, 0.0f }, {0, 0} },
	{ { 1.0f, 1.0f, 0.0f }, {1, 0} },
	{ { 1.0f,-1.0f, 0.0f }, {1, 1} },
	{ {-1.0f,-1.0f, 0.0f }, {0, 1} },
	};

	m_vertexBuffer = dxgl::DXGLMain::resource()->createVertexBuffer(&vertices[0], vertices.size(), sizeof(PostProcessVertex));

	std::vector<unsigned int> indices = {
		0, 1, 2, 2, 3, 0
	};

	m_indexBuffer = dxgl::DXGLMain::resource()->createIndexBuffer(&indices[0], indices.size());

	m_pscbScreen = dxgl::DXGLMain::resource()->createPSConstantBuffer(sizeof(ScreenBuffer));

	m_vs = dxgl::DXGLMain::resource()->createShader<dxgl::DXGLVertexShader>("Assets/Shaders/VS_PostProcess.cso");
	m_ps = dxgl::DXGLMain::resource()->createShader<dxgl::DXGLPixelShader>("Assets/Shaders/PS_PostProcess.cso");
	m_psDrunk = dxgl::DXGLMain::resource()->createShader<dxgl::DXGLPixelShader>("Assets/Shaders/PS_DrunkVision.cso");
	m_psNight = dxgl::DXGLMain::resource()->createShader<dxgl::DXGLPixelShader>("Assets/Shaders/PS_NightVision.cso");
	m_psPixel = dxgl::DXGLMain::resource()->createShader<dxgl::DXGLPixelShader>("Assets/Shaders/PS_Pixelated.cso");

}

PostProcessor::~PostProcessor() {

}

void PostProcessor::update(long double delta, float width, float height) {

	// screen
	ScreenBuffer screen {
		width,
		height,
	};

	m_pscbScreen->update(&screen);

	// use no vision
	if (dxgl::DXGLMain::input()->getKeyTapState('1')) {
		m_pscbEffect.reset();
		m_mode = 0;
	}

	// use drunk vision
	if (dxgl::DXGLMain::input()->getKeyTapState('2')) {
		m_pscbEffect.reset();
		m_pscbEffect = dxgl::DXGLMain::resource()->createPSConstantBuffer(sizeof(DrunkVisionBuffer));
		m_mode = 1;
	}

	// use night vision
	if (dxgl::DXGLMain::input()->getKeyTapState('3')) {
		m_pscbEffect.reset();
		m_pscbEffect = dxgl::DXGLMain::resource()->createPSConstantBuffer(sizeof(NightVisionBuffer));
		m_mode = 2;
	}

	// use pixel vision
	if (dxgl::DXGLMain::input()->getKeyTapState('4')) {
		m_pscbEffect.reset();
		m_mode = 3;
	}

	if (m_mode == 1) {
		DrunkVisionBuffer dvbuff{};
		m_timePassed += delta;
		dvbuff.time = m_timePassed;
		dvbuff.period = 8.0f;
		dvbuff.amplitude = 0.0125f;
		m_pscbEffect->update(&dvbuff);
	}

	if (m_mode == 2) {
		NightVisionBuffer nvbuff{};
		m_timePassed += delta;
		nvbuff.time = m_timePassed;
		nvbuff.speed = 1.0f;
		nvbuff.amplitude = 0.00125f;
		m_pscbEffect->update(&nvbuff);
	}
}

void PostProcessor::draw() {
	dxgl::SP_DXGLShaderResourceView post = dxgl::DXGLMain::renderer()->getRenderToTexture(dxgl::RESOURCE_VIEW_SLOT_BACK_BUFFER);
	float color[4] = {0, 0, 0, 1};
	dxgl::SP_DXGLRenderTargetView rtv = dxgl::DXGLMain::renderer()->getRTV(dxgl::RESOURCE_VIEW_SLOT_BACK_BUFFER);
	dxgl::SP_DXGLDepthStencilView dsv = dxgl::DXGLMain::renderer()->getDSV(dxgl::RESOURCE_VIEW_SLOT_BACK_BUFFER);
	dxgl::DXGLMain::renderer()->setRenderTarget(rtv, color, dsv);

	m_inputLayout->bind();
	m_vertexBuffer->bind(0);
	m_indexBuffer->bind();

	dxgl::DXGLMain::renderer()->shader()->VS_setCBuffer(0, 1, 0);

	dxgl::DXGLMain::renderer()->shader()->PS_setCBuffer(0, 1, m_pscbScreen->get());

	if (m_pscbEffect && m_pscbEffect->get()) {
		dxgl::DXGLMain::renderer()->shader()->PS_setCBuffer(1, 1, m_pscbEffect->get());
	} else {
		dxgl::DXGLMain::renderer()->shader()->PS_setCBuffer(1, 1, 0);
	}

	dxgl::DXGLMain::renderer()->shader()->VS_setShader(m_vs);
	dxgl::DXGLMain::renderer()->shader()->HS_setShader(nullptr);
	dxgl::DXGLMain::renderer()->shader()->DS_setShader(nullptr);

	switch (m_mode) {
	case 0:
		dxgl::DXGLMain::renderer()->shader()->PS_setShader(m_ps);
		break;
	case 1:
		dxgl::DXGLMain::renderer()->shader()->PS_setShader(m_psDrunk);
		break;
	case 2:
		dxgl::DXGLMain::renderer()->shader()->PS_setShader(m_psNight);
		break;
	case 3:
		dxgl::DXGLMain::renderer()->shader()->PS_setShader(m_psPixel);
		break;
	}

	dxgl::DXGLMain::renderer()->shader()->PS_setResource(0, post->get());
	dxgl::DXGLMain::renderer()->shader()->PS_setResource(1, 0);
	dxgl::DXGLMain::renderer()->shader()->PS_setResource(2, 0);
	dxgl::DXGLMain::renderer()->shader()->PS_setResource(3, 0);
	dxgl::DXGLMain::renderer()->shader()->PS_setResource(4, 0);
	dxgl::DXGLMain::renderer()->shader()->PS_setResource(5, 0);
	dxgl::DXGLMain::renderer()->shader()->PS_setResource(6, 0);
	dxgl::DXGLMain::renderer()->shader()->PS_setResource(7, 0);
	dxgl::DXGLMain::renderer()->shader()->PS_setResource(8, 0);
	dxgl::DXGLMain::renderer()->shader()->PS_setResource(9, 0);

	dxgl::DXGLMain::renderer()->drawIndexedTriangleList(6, 0, 0);
}
