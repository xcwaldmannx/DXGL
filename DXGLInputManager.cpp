#include "DXGLInputManager.h"

using namespace dxgl;

DXGLInputManager::DXGLInputManager() {
	for (int i = 0; i < 256; i++) {
		m_keyHoldStates[i] = 0.0f;
		m_keyPressStates[i] = 0.0f;
		m_keyTapStates[i] = 0.0f;
	}

	m_lmbState = 0.0f;
	m_mmbState = 0.0f;
	m_rmbState = 0.0f;
}

DXGLInputManager::~DXGLInputManager() {
}

bool DXGLInputManager::getKeyHoldState(char key, float duration) {
	return m_keyHoldStates[key] >= duration;
}

float DXGLInputManager::getKeyPressState(char key) {
	return m_keyPressStates[key];
}

float DXGLInputManager::getKeyTapState(char key) {
	const float value = m_keyTapStates[key];
	m_keyTapStates[key] = 0;
	return value;
}

float DXGLInputManager::getMouseState(MOUSE_STATE state) {
	switch (state) {
	case LMB_STATE:
		return m_lmbState;
	case MMB_STATE:
		return m_mmbState;
	case RMB_STATE:
		return m_rmbState;
	default:
		return 0.0f;
	}
}

const Point2f& DXGLInputManager::getMousePosition() {
	return m_position;
}

const Point2f& DXGLInputManager::getMouseDelta() {
	const Point2f output = m_delta;
	m_delta = { 0, 0 };
	return output;
}

void DXGLInputManager::onKeyDown(int key) {
	m_keyHoldStates[key]++;
	m_keyPressStates[key] = 1.0f;
	if (m_keyTapStates[key] == -1) {
		m_keyTapStates[key] = 1.0f;
	}
}

void DXGLInputManager::onKeyUp(int key) {
	m_keyHoldStates[key] = 0.0f;
	m_keyPressStates[key] = 0.0f;
	m_keyTapStates[key] = -1.0f;
}

void DXGLInputManager::onMouseMove(const Point2f& mousePos, const Point2f& mouseDelta) {
	m_position.x = mousePos.x;
	m_position.y = mousePos.y;
	m_delta.x = mouseDelta.x;
	m_delta.y = mouseDelta.y;
}

void DXGLInputManager::onLeftMouseDown(const Point2f& mousePos) {
	m_lmbState = 1.0f;
}

void DXGLInputManager::onLeftMouseUp(const Point2f& mousePos) {
	m_lmbState = 0.0f;
}

void DXGLInputManager::onRightMouseDown(const Point2f& mousePos) {
	m_rmbState = 1.0f;
}

void DXGLInputManager::onRightMouseUp(const Point2f& mousePos) {
	m_rmbState = 0.0f;
}
