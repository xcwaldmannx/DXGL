#pragma once

class Point2f {
public:
	Point2f() : x{ 0 }, y{ 0 } {

	}

	Point2f(float x, float y) : x{ x }, y{ y } {

	}

public:
	float x;
	float y;
};