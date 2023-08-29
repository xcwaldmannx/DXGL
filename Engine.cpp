#include "Engine.h"

#include <iostream>

using namespace dxgl;


SP_Engine Engine::m_window = nullptr;

Engine::Engine() {
	DXGLInputSystem::create(m_hwnd);
	m_window = SP_Engine(this, [](Engine*) {});
}

Engine::~Engine() {
	DXGLInputSystem::destroy();
}

SP_Engine Engine::window() {
	return m_window;
}

SP_DXGLGraphics Engine::m_graphics = std::make_shared<DXGLGraphics>();
SP_DXGLGraphics Engine::graphics() {
	return Engine::m_graphics;
}

SP_Renderer Engine::m_renderer = std::make_shared<Renderer>();
SP_Renderer Engine::renderer() {
	return Engine::m_renderer;
}

SP_DXGLResourceManager Engine::m_resource = std::make_shared<DXGLResourceManager>();
SP_DXGLResourceManager Engine::resource() {
	return Engine::m_resource;
}

SP_EntityManager Engine::m_entities = std::make_shared<EntityManager>();
SP_EntityManager Engine::entities() {
	return Engine::m_entities;
}

SP_PhysicsManager Engine::m_physics = std::make_shared<PhysicsManager>();
SP_PhysicsManager Engine::physics() {
	return Engine::m_physics;
}

SP_CameraManager Engine::m_camera = std::make_shared<CameraManager>();
SP_CameraManager Engine::camera() {
	return Engine::m_camera;
}

SP_MousePickManager Engine::m_mousePick = std::make_shared<MousePickManager>();
SP_MousePickManager Engine::mousePick() {
	return Engine::m_mousePick;
}

SP_DXGLInputManager Engine::m_userInput = std::make_shared<DXGLInputManager>();
SP_DXGLInputManager Engine::input() {
	return m_userInput;
}

void Engine::enableVSync(bool enable) {
	m_vsyncEnabled = enable;
}

void Engine::onCreate() {
	DXGLWindow::onCreate();


	renderer()->createSwapchain(this);

	m_userInput = std::make_shared<DXGLInputManager>();
	DXGLInputSystem::get()->addListener(m_userInput.get());
	create();

	m_lastUpdate = std::chrono::steady_clock::now();

	renderer()->init();
}

void Engine::onUpdate() {
	auto now = std::chrono::steady_clock::now();

	DXGLWindow::onUpdate();

	// update input events
	DXGLInputSystem::get()->update();

	// update managers
	camera()->update(m_deltaTime);

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

void Engine::onDestroy() {
	DXGLWindow::onDestroy();
}

void Engine::onFocus() {
	DXGLWindow::onFocus();
	DXGLInputSystem::get()->addListener(m_userInput.get());
}

void Engine::onKillFocus() {
	DXGLWindow::onKillFocus();
	DXGLInputSystem::get()->removeListener(m_userInput.get());
}

void Engine::onMove() {
	DXGLWindow::onMove();
	onUpdate();
}

void Engine::onSize() {
	DXGLWindow::onSize();
	RECT dim = getWindowSize();
	renderer()->resizeSwapchain(dim.right - dim.left, dim.bottom - dim.top);
	onUpdate();
}

void Engine::quit() {
	onDestroy();
}