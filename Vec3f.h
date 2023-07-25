#pragma once

#include <cmath>
#include <string>

#include "Vec2f.h"

class Vec3f {
public:


	Vec3f() : x(0), y(0), z(0) {
	}

	Vec3f(float f) : x(f), y(f), z(f) {
	}

	Vec3f(float x, float y, float z) : x(x), y(y), z(z) {
	}

	Vec3f(const Vec3f& vector) : x(vector.x), y(vector.y), z(vector.z) {
	}

	Vec3f(const Vec2f& vector) : x(vector.x), y(vector.y), z(1.0f) {
	}

	~Vec3f() {

	}

	static Vec3f lerp(const Vec3f& start, const Vec3f& end, float delta) {
		Vec3f result;
		result.x = start.x * (1.0f - delta) + end.x * (delta);
		result.y = start.y * (1.0f - delta) + end.y * (delta);
		result.z = start.z * (1.0f - delta) + end.z * (delta);
		return result;
	}

	Vec3f normalize() {
		Vec3f result{};
		float value = (float)sqrt(x * x + y * y + z * z);
		result.x = x / value;
		result.y = y / value;
		result.z = z / value;
		return result;
	}

	static Vec3f normalize(const Vec3f& vec) {
		Vec3f result{};
		float value = (float)sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
		result.x = vec.x / value;
		result.y = vec.y / value;
		result.z = vec.z / value;
		return result;
	}

	float magnitude() {
		return (float)sqrt(x * x + y * y + z * z);
	}

	static float dot(Vec3f v1, Vec3f v2) {
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	static Vec3f cross(Vec3f v1, Vec3f v2) {
		float x = (v1.y * v2.z) - (v2.y * v1.z);
		float y = -((v1.x * v2.z) - (v2.x * v1.z));
		float z = (v1.x * v2.y) - (v2.x * v1.y);
		return Vec3f(x, y, z);
	}

	static float dist(Vec3f v1, Vec3f v2) {
		float d1 = (v2.x - v1.x) * (v2.x - v1.x);
		float d2 = (v2.y - v1.y) * (v2.y - v1.y);
		float d3 = (v2.z - v1.z) * (v2.z - v1.z);
		return (float)sqrt(d1 + d2 + d3);
	}

	static float angleBetween(Vec3f v1, Vec3f v2) {
		float dotProduct = dot(v1, v2);
		float mag1 = v1.magnitude();
		float mag2 = v2.magnitude();
		return acos(dotProduct / (mag1 * mag2));
	}

	Vec3f operator+ (Vec3f vec) {
		return Vec3f(x + vec.x, y + vec.y, z + vec.z);
	}

	void operator+= (Vec3f vec) {
		x += vec.x;
		y += vec.y;
		z += vec.z;
	}

	Vec3f operator- (Vec3f vec) {
		return Vec3f(x - vec.x, y - vec.y, z - vec.z);
	}

	void operator-= (Vec3f vec) {
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
	}

	Vec3f operator* (float num) {
		return Vec3f(x * num, y * num, z * num);
	}

	Vec3f operator* (Vec3f vec) {
		return Vec3f(x * vec.x, y * vec.y, z * vec.z);
	}

	void operator*= (Vec3f vec) {
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
	}

	Vec3f operator/ (float num) {
		return Vec3f(x / num, y / num, z / num);
	}

	bool operator== (Vec3f vec) {
		return x == vec.x && y == vec.y && z == vec.z;
	}

	std::string toString() {
		return std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z);
	}

	float x = 0;
	float y = 0;
	float z = 0;
};
