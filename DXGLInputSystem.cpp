#include "DXGLInputSystem.h"

#include <exception>

#include "DXGLWindow.h"

using namespace dxgl;

DXGLInputSystem* DXGLInputSystem::m_inputSystem = nullptr;

DXGLInputSystem::DXGLInputSystem(HWND& window) : m_window(window) {
}

DXGLInputSystem::~DXGLInputSystem() {
	m_inputSystem = nullptr;
}

void DXGLInputSystem::update() {

	POINT currentMousePos{};
	GetCursorPos(&currentMousePos);

	if (m_firstTime) {
		m_oldMousePos = { (float) currentMousePos.x, (float) currentMousePos.y };
		m_firstTime = false;
	}

	if (currentMousePos.x != m_oldMousePos.x || currentMousePos.y != m_oldMousePos.y) {
		// Mouse move event
		std::unordered_set<DXGLInputListener*>::iterator it = m_listeners.begin();
		while (it != m_listeners.end()) {
			(*it)->onMouseMove(Point2f((float)currentMousePos.x, (float)currentMousePos.y),
				Point2f((float)currentMousePos.x - m_oldMousePos.x, (float)currentMousePos.y - m_oldMousePos.y));
			it++;
		}
	}
	m_oldMousePos = { (float) currentMousePos.x, (float) currentMousePos.y };

	if (GetKeyboardState(m_keysState)) {
		for (unsigned int i = 0; i < 256; i++) {
			// Key is down
			if (m_keysState[i] & 0x80) {
				std::unordered_set<DXGLInputListener*>::iterator it = m_listeners.begin();
				while (it != m_listeners.end()) {

					// LEFT MOUSE
					if (i == VK_LBUTTON) {
						if (m_keysState[i] != m_keysStateOld[i]) {
							(*it)->onLeftMouseDown(Point2f((float)currentMousePos.x, (float)currentMousePos.y));
						}
					}
					// RIGHT MOUSE
					else if (i == VK_RBUTTON) {
						if (m_keysState[i] != m_keysStateOld[i]) {
							(*it)->onRightMouseDown(Point2f((float)currentMousePos.x, (float)currentMousePos.y));
						}
					}
					// KEYBOARD
					else {
						(*it)->onKeyDown(i);
					}
					it++;
				}
			}
			// Key is up
			else {
				if (m_keysState[i] != m_keysStateOld[i]) {
					std::unordered_set<DXGLInputListener*>::iterator it = m_listeners.begin();
					while (it != m_listeners.end()) {
						// LEFT MOUSE
						if (i == VK_LBUTTON) {
							(*it)->onLeftMouseUp(Point2f((float)currentMousePos.x, (float)currentMousePos.y));
						}
						// RIGHT MOUSE
						else if (i == VK_RBUTTON) {
							(*it)->onRightMouseUp(Point2f((float)currentMousePos.x, (float)currentMousePos.y));
						}
						// KEYBOARD
						else {
							(*it)->onKeyUp(i);
						}
						it++;
					}
				}
			}
		}
		// store current keys state
		memcpy(m_keysStateOld, m_keysState, sizeof(unsigned char) * 256);
	}
}

void DXGLInputSystem::addListener(DXGLInputListener* listener) {
	m_listeners.insert(listener);
}

void DXGLInputSystem::removeListener(DXGLInputListener* listener) {
	m_listeners.erase(listener);
}

void DXGLInputSystem::setCursorPosition(const Point2f& pos) {
	SetCursorPos((int)pos.x, (int)pos.y);
	m_oldMousePos = { pos.x, pos.y };
}

void DXGLInputSystem::showCursor(bool show) {
	ShowCursor(show);
}

void DXGLInputSystem::create(HWND& window) {
	if (DXGLInputSystem::m_inputSystem) {
		throw std::exception("DXGLInputSystem already exists.");
	}
	DXGLInputSystem::m_inputSystem = new DXGLInputSystem(window);
}

void DXGLInputSystem::destroy() {
	if (!DXGLInputSystem::m_inputSystem) {
		return;
	}
	delete DXGLInputSystem::m_inputSystem;
}

DXGLInputSystem* DXGLInputSystem::get() {
	return m_inputSystem;
}
