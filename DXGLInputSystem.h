#pragma once

#include <unordered_set>
#include <Windows.h>

#include "DXGLInputListener.h"

namespace dxgl {
	class DXGLInputSystem {
	private:
		DXGLInputSystem(HWND& window);
		~DXGLInputSystem();
	public:
		void update();
		void addListener(DXGLInputListener* listener);
		void removeListener(DXGLInputListener* listener);

		void setCursorPosition(const Point2f& pos);
		void showCursor(bool show);

		static void create(HWND& window);
		static void destroy();
		static DXGLInputSystem* get();

	private:
		HWND m_window;

		static DXGLInputSystem* m_inputSystem;

		std::unordered_set<DXGLInputListener*> m_listeners{};
		unsigned char m_keysState[256] = {};
		unsigned char m_keysStateOld[256] = {};

		Point2f m_oldMousePos{ 0, 0 };
		bool m_firstTime = true;
	};
}
