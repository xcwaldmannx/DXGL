#include "Engine.h"

#include <iostream>

#include "Graphics.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "TextRenderManager.h"
#include "EntityManager.h"
#include "MousePickManager.h"
#include "PhysicsManager.h"
#include "ControllerManager.h"
#include "CameraManager.h"

#include "InputSystem.h"
#include "InputManager.h"

using namespace dxgl;


SP_Engine Engine::m_window = nullptr;

Engine::Engine() {
	InputSystem::create(m_hwnd);
	m_window = SP_Engine(this, [](Engine*) {});
}

Engine::~Engine() {
	InputSystem::destroy();
}

SP_Engine Engine::window() {
	return m_window;
}

SP_Graphics Engine::m_graphics = std::make_shared<Graphics>();
SP_Graphics Engine::graphics() {
	return Engine::m_graphics;
}

SP_Renderer Engine::m_renderer = std::make_shared<Renderer>();
SP_Renderer Engine::renderer() {
	return Engine::m_renderer;
}

SP_ResourceManager Engine::m_resource = std::make_shared<ResourceManager>();
SP_ResourceManager Engine::resource() {
	return Engine::m_resource;
}

SP_TextRenderManager Engine::m_textrender = std::make_shared<TextRenderManager>();
SP_TextRenderManager Engine::textrender() {
	return Engine::m_textrender;
}

SP_EntityManager Engine::m_entities = std::make_shared<EntityManager>();
SP_EntityManager Engine::entities() {
	return Engine::m_entities;
}

SP_PhysicsManager Engine::m_physics = std::make_shared<PhysicsManager>();
SP_PhysicsManager Engine::physics() {
	return Engine::m_physics;
}

SP_ControllerManager Engine::m_controller = std::make_shared<ControllerManager>();
SP_ControllerManager Engine::controller() {
	return Engine::m_controller;
}

SP_CameraManager Engine::m_camera = std::make_shared<CameraManager>();
SP_CameraManager Engine::camera() {
	return Engine::m_camera;
}

SP_MousePickManager Engine::m_mousePick = std::make_shared<MousePickManager>();
SP_MousePickManager Engine::mousePick() {
	return Engine::m_mousePick;
}

SP_InputManager Engine::m_userInput = std::make_shared<InputManager>();
SP_InputManager Engine::input() {
	return m_userInput;
}

int Engine::m_avgFPS = 0;

void Engine::enableVSync(bool enable) {
	m_vsyncEnabled = enable;
}

void Engine::onCreate() {
	DXGLWindow::onCreate();


	renderer()->createSwapchain(this);

	InputSystem::get()->addListener(m_userInput.get());

	create();

	m_lastUpdate = std::chrono::steady_clock::now();

	renderer()->init();
}

void Engine::onUpdate() {
	auto now = std::chrono::steady_clock::now();

	DXGLWindow::onUpdate();

	// update input events
	InputSystem::get()->update();

	// update managers
	controller()->update(m_deltaTime);
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
		m_avgFPS = m_FPS;
		m_FPS = 0;
		m_avgMs = 0;
		m_timeOneSecond = 0;
	}
}

int Engine::getAverageFPS() {
	return Engine::m_avgFPS;
}

void Engine::onDestroy() {
	DXGLWindow::onDestroy();
}

void Engine::onFocus() {
	DXGLWindow::onFocus();
	InputSystem::get()->addListener(m_userInput.get());
}

void Engine::onKillFocus() {
	DXGLWindow::onKillFocus();
	InputSystem::get()->removeListener(m_userInput.get());
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