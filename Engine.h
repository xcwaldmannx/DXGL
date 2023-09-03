#pragma once

#include <chrono>

#include "DXGLWindow.h"

#include "DXGLDefinitions.h"

class dxgl::Graphics;
class dxgl::Renderer;
class dxgl::ResourceManager;
class dxgl::TextRenderManager;
class dxgl::EntityManager;
class dxgl::MousePickManager;
class dxgl::PhysicsManager;
class dxgl::CameraManager;
class dxgl::ControllerManager;


namespace dxgl {

	class Engine : public DXGLWindow {
	public:
		Engine();
		~Engine();

		static SP_Engine            window();
		static SP_Graphics          graphics();
		static SP_Renderer          renderer();
		static SP_ResourceManager   resource();
		static SP_TextRenderManager textrender();
		static SP_EntityManager     entities();
		static SP_PhysicsManager    physics();
		static SP_ControllerManager controller();
		static SP_CameraManager     camera();
		static SP_MousePickManager  mousePick();
		static SP_InputManager      input();

		static int getAverageFPS();

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
		static SP_Graphics            m_graphics;
		static SP_Renderer            m_renderer;
		static SP_ResourceManager     m_resource;
		static SP_TextRenderManager   m_textrender;
		static SP_EntityManager       m_entities;
		static SP_PhysicsManager      m_physics;
		static SP_ControllerManager   m_controller;
		static SP_CameraManager       m_camera;
		static SP_MousePickManager    m_mousePick;
		static SP_InputManager        m_userInput;


		bool m_vsyncEnabled = true;

		long double m_deltaTime = 0.0;
		std::chrono::steady_clock::time_point m_lastUpdate{};

		long double m_timeOneSecond = 0;
		int m_FPS;
		static int m_avgFPS;
		float m_avgMs = 0;

	};

}