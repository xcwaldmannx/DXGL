#include "InputManager.h"

using namespace dxgl;

InputManager::InputManager() {
	for (int i = 0; i < 256; i++) {
		m_keyHoldStates[i] = 0;
		m_keyPressStates[i] = 0;
		m_keyTapStates[i] = 0;
		m_keyReleaseStates[i] = 0;
	}

	m_lmbState = 0;
	m_mmbState = 0;
	m_rmbState = 0;
}

InputManager::~InputManager() {
}

bool InputManager::getKeyHoldState(char key, float duration) {
	return m_keyHoldStates[key] >= duration;
}

bool InputManager::getKeyPressState(char key) {
	return m_keyPressStates[key];
}

bool InputManager::getKeyTapState(char key) {
	int value = m_keyTapStates[key];

	if (value == 1) {
		m_keyTapStates[key] = 2;
		return true;
	}

	return false;
}

bool InputManager::getKeyReleaseState(char key) {
	int value = m_keyReleaseStates[key];

	if (value == 1) {
		m_keyReleaseStates[key] = 2;
		return true;
	}

	return false;
}

float InputManager::getMouseState(MOUSE_STATE state) {
	switch (state) {
	case LMB_STATE:
		return m_lmbState;
	case MMB_STATE:
		return m_mmbState;
	case RMB_STATE:
		return m_rmbState;
	default:
		return 0;
	}
}

const Point2f& InputManager::getMousePosition() {
	return m_position;
}

const Point2f& InputManager::getMouseDelta() {
	const Point2f output = m_delta;
	m_delta = { 0, 0 };
	return output;
}

void InputManager::onKeyDown(int key) {
	m_keyHoldStates[key]++;
	m_keyPressStates[key] = 1;
	if (m_keyTapStates[key] == 0) m_keyTapStates[key] = 1;
	m_keyReleaseStates[key] = 0;
}

void InputManager::onKeyUp(int key) {
	m_keyHoldStates[key] = 0;
	m_keyPressStates[key] = 0;
	m_keyTapStates[key] = 0;
	if (m_keyReleaseStates[key] == 0) m_keyReleaseStates[key] = 1;
}

void InputManager::onMouseMove(const Point2f& mousePos, const Point2f& mouseDelta) {
	m_position.x = mousePos.x;
	m_position.y = mousePos.y;
	m_delta.x = mouseDelta.x;
	m_delta.y = mouseDelta.y;
}

void InputManager::onLeftMouseDown(const Point2f& mousePos) {
	m_lmbState = 1.0f;
}

void InputManager::onLeftMouseUp(const Point2f& mousePos) {
	m_lmbState = 0.0f;
}

void InputManager::onRightMouseDown(const Point2f& mousePos) {
	m_rmbState = 1.0f;
}

void InputManager::onRightMouseUp(const Point2f& mousePos) {
	m_rmbState = 0.0f;
}
