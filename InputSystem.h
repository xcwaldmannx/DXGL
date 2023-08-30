#pragma once

#include <unordered_set>
#include <Windows.h>

#include "InputListener.h"

namespace dxgl {
	class InputSystem {
	private:
		InputSystem(HWND& window);
		~InputSystem();
	public:
		void update();
		void addListener(InputListener* listener);
		void removeListener(InputListener* listener);

		void setCursorPosition(const Point2f& pos);
		void showCursor(bool show);

		static void create(HWND& window);
		static void destroy();
		static InputSystem* get();

	private:
		HWND m_window;

		static InputSystem* m_inputSystem;

		std::unordered_set<InputListener*> m_listeners{};
		unsigned char m_keysState[256] = {};
		unsigned char m_keysStateOld[256] = {};

		Point2f m_oldMousePos{ 0, 0 };
		bool m_firstTime = true;
	};
}
