#pragma once

#include "Vec2f.h"
#include "Vec3f.h"

class Vec4f {
public:
	Vec4f() : x(0), y(0), z(0), w(0) {
	}

	Vec4f(float f) : x(f), y(f), z(f), w(f) {
	}

	Vec4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {
	}

	Vec4f(const Vec4f& vector) : x(vector.x), y(vector.y), z(vector.z), w(vector.w) {
	}

	Vec4f(const Vec3f& vector) : x(vector.x), y(vector.y), z(vector.z), w(1.0f) {
	}

	Vec4f(const Vec2f& vector) : x(vector.x), y(vector.y), z(1.0f), w(1.0f) {
	}

	~Vec4f() {

	}

	//static Vec4f lerp(const Vec4f& start, const Vec4f& end, float delta) {
	//	Vec4f result;
	//	result.x = start.x * (1.0f - delta) + end.x * (delta);
	//	result.y = start.y * (1.0f - delta) + end.y * (delta);
	//	result.z = start.z * (1.0f - delta) + end.z * (delta);
	//	result.w = start.w * (1.0f - delta) + end.w * (delta);
	//	return result;
	//}

	void cross(Vec4f& v1, Vec4f& v2, Vec4f& v3) {
		this->x = v1.y * (v2.z * v3.w - v3.z * v2.w) - v1.z * (v2.y * v3.w - v3.y * v2.w) + v1.w * (v2.y * v3.z - v2.z * v3.y);
		this->y = -(v1.x * (v2.z * v3.w - v3.z * v2.w) - v1.z * (v2.x * v3.w - v3.x * v2.w) + v1.w * (v2.x * v3.z - v3.x * v2.z));
		this->z = v1.x * (v2.y * v3.w - v3.y * v2.w) - v1.y * (v2.x * v3.w - v3.x * v2.w) + v1.w * (v2.x * v3.y - v3.x * v2.y);
		this->w = -(v1.x * (v2.y * v3.z - v3.y * v2.z) - v1.y * (v2.x * v3.z - v3.x * v2.z) + v1.z * (v2.x * v3.y - v3.x * v2.y));
	}

	Vec3f xyz() {
		return Vec3f{x, y, z};
	}

	float x = 0;
	float y = 0;
	float z = 0;
	float w = 0;
};