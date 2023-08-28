#include "DXGLMain.h"

#include <iostream>

using namespace dxgl;


SP_DXGLMain DXGLMain::m_window = nullptr;

DXGLMain::DXGLMain() {
	DXGLInputSystem::create(m_hwnd);
	m_window = SP_DXGLMain(this, [](DXGLMain*) {});
}

DXGLMain::~DXGLMain() {
	DXGLInputSystem::destroy();
}

SP_DXGLMain DXGLMain::window() {
	return m_window;
}

SP_DXGLGraphics DXGLMain::m_graphics = std::make_shared<DXGLGraphics>();
SP_DXGLGraphics DXGLMain::graphics() {
	return DXGLMain::m_graphics;
}

SP_DXGLRenderer DXGLMain::m_renderer = std::make_shared<DXGLRenderer>();
SP_DXGLRenderer DXGLMain::renderer() {
	return DXGLMain::m_renderer;
}

SP_DXGLResourceManager DXGLMain::m_resource = std::make_shared<DXGLResourceManager>();
SP_DXGLResourceManager DXGLMain::resource() {
	return DXGLMain::m_resource;
}

SP_EntityManager DXGLMain::m_entities = std::make_shared<EntityManager>();
SP_EntityManager DXGLMain::entities() {
	return DXGLMain::m_entities;
}

SP_PhysicsManager DXGLMain::m_physics = std::make_shared<PhysicsManager>();
SP_PhysicsManager DXGLMain::physics() {
	return DXGLMain::m_physics;
}

SP_MousePickManager DXGLMain::m_mousePick = std::make_shared<MousePickManager>();
SP_MousePickManager DXGLMain::mousePick() {
	return DXGLMain::m_mousePick;
}

SP_DXGLInputManager DXGLMain::m_userInput = std::make_shared<DXGLInputManager>();
SP_DXGLInputManager DXGLMain::input() {
	return m_userInput;
}

void DXGLMain::enableVSync(bool enable) {
	m_vsyncEnabled = enable;
}

void DXGLMain::onCreate() {
	DXGLWindow::onCreate();


	renderer()->createSwapchain(this);

	m_userInput = std::make_shared<DXGLInputManager>();
	DXGLInputSystem::get()->addListener(m_userInput.get());
	create();

	m_lastUpdate = std::chrono::steady_clock::now();

	renderer()->init();
}

void DXGLMain::onUpdate() {
	auto now = std::chrono::steady_clock::now();

	DXGLWindow::onUpdate();

	// update input events
	DXGLInputSystem::get()->update();

	// update call
	update(m_deltaTime);

	if (!renderer()->isPaused()) {
		// draw call
		draw();

		// present swapchain buffer
		renderer()->present(m_vsyncEnabled);
	}

	m_deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_lastUpdate).count() / 1000000000.0;
	m_avgMs += std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_lastUpdate).count() / 1000000.0;
	m_lastUpdate = now;

	m_FPS++;

	m_timeOneSecond += m_deltaTime;
	if (m_timeOneSecond > 1) {
		m_avgMs /= m_FPS;
		std::cout << "FPS: " << m_FPS << ", " << m_avgMs << "ms" << "\n";
		m_FPS = 0;
		m_avgMs = 0;
		m_timeOneSecond = 0;
	}
}

void DXGLMain::onDestroy() {
	DXGLWindow::onDestroy();
}

void DXGLMain::onFocus() {
	DXGLWindow::onFocus();
	DXGLInputSystem::get()->addListener(m_userInput.get());
}

void DXGLMain::onKillFocus() {
	DXGLWindow::onKillFocus();
	DXGLInputSystem::get()->removeListener(m_userInput.get());
}

void DXGLMain::onMove() {
	DXGLWindow::onMove();
	onUpdate();
}

void DXGLMain::onSize() {
	DXGLWindow::onSize();
	RECT dim = getWindowSize();
	renderer()->resizeSwapchain(dim.right - dim.left, dim.bottom - dim.top);
	onUpdate();
}

void DXGLMain::quit() {
	onDestroy();
}