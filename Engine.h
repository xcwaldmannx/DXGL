#pragma once

#include <chrono>

#include "DXGLDefinitions.h"

#include "DXGLGraphics.h"
#include "SwapChain.h"
#include "Renderer.h"
#include "EntityManager.h"
#include "PhysicsManager.h"
#include "MousePickManager.h"
#include "DXGLResourceManager.h"

#include "DXGLWindow.h"
#include "DXGLInputSystem.h"
#include "DXGLInputManager.h"

namespace dxgl {

	class Engine : public DXGLWindow {
	public:
		Engine();
		~Engine();

		static SP_Engine              window();
		static SP_DXGLGraphics        graphics();
		static SP_Renderer            renderer();
		static SP_DXGLResourceManager resource();
		static SP_EntityManager       entities();
		static SP_PhysicsManager      physics();
		static SP_MousePickManager    mousePick();
		static SP_DXGLInputManager    input();

		void enableVSync(bool enable);

		void onCreate() override;
		void onUpdate() override;
		void onDestroy() override;
		void onFocus() override;
		void onKillFocus() override;
		void onMove() override;
		void onSize() override;

		void quit();

		virtual void create() = 0;
		virtual void update(long double deltaTime) = 0;
		virtual void draw() = 0;

	private:
		static SP_Engine              m_window;
		static SP_DXGLGraphics        m_graphics;
		static SP_Renderer            m_renderer;
		static SP_DXGLResourceManager m_resource;
		static SP_EntityManager       m_entities;
		static SP_PhysicsManager      m_physics;
		static SP_MousePickManager    m_mousePick;
		static SP_DXGLInputManager    m_userInput;


		bool m_vsyncEnabled = true;

		long double m_deltaTime = 0.0;
		std::chrono::steady_clock::time_point m_lastUpdate{};

		long double m_timeOneSecond = 0;
		int m_FPS = 0;
		float m_avgMs = 0;

	};

}