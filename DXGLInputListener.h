#pragma once

#include "Point2f.h"

class DXGLInputListener {
public:
	DXGLInputListener() {
	}

	~DXGLInputListener() {
	}

	// Keyboard
	virtual void onKeyDown(int key) = 0;
	virtual void onKeyUp(int key) = 0;

	// Mouse
	virtual void onMouseMove(const Point2f& mousePos, const Point2f& mouseDelta) = 0;

	virtual void onLeftMouseDown(const Point2f& mousePos) = 0;
	virtual void onLeftMouseUp(const Point2f& mousePos) = 0;

	virtual void onRightMouseDown(const Point2f& mousePos) = 0;
	virtual void onRightMouseUp(const Point2f& mousePos) = 0;

};