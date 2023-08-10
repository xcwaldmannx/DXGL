#pragma once

class Point3f {
public:
	Point3f() : x{ 0 }, y{ 0 }, z{ 0 } {

	}

	Point3f(float x, float y, float z) : x{ x }, y{ y }, z{ z } {

	}

public:
	float x;
	float y;
	float z;
};