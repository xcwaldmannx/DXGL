#pragma once

class Vec2f {
public:
	Vec2f() : x(0), y(0) {
	}


	Vec2f(float x, float y) : x(x), y(y) {
	}

	Vec2f(const Vec2f& vec) : x(vec.x), y(vec.y) {
	}

	~Vec2f() {
	}

	static float length(Vec2f vec) {
		return sqrt(vec.x * vec.x + vec.y * vec.y);
	}

	static Vec2f normalize(Vec2f vec) {
		return vec / length(vec);
	}

	static float det(Vec2f v1, Vec2f v2) {
		return (v1.x * v2.y) - (v2.x * v1.y);
	}

	static float cross(Vec2f v1, Vec2f v2) {
		return (v1.x * v2.y - v1.y * v2.x);
	}

	Vec2f operator+ (Vec2f vec) {
		return Vec2f(x + vec.x, y + vec.y);
	}


	void operator+= (Vec2f vec) {
		x += vec.x;
		y += vec.y;
	}

	Vec2f operator- (Vec2f vec) {
		return Vec2f(x - vec.x, y - vec.y);
	}

	void operator-= (Vec2f vec) {
		x -= vec.x;
		y -= vec.y;
	}

	Vec2f operator* (Vec2f vec) {
		return Vec2f(vec.x * vec.x, vec.y * vec.y);
	}

	Vec2f operator* (float num) {
		return Vec2f(x * num, y * num);
	}

	void operator*= (Vec2f vec) {
		x *= vec.x;
		y *= vec.y;
	}

	Vec2f operator/ (float num) {
		return Vec2f(x / num, y / num);
	}

	void operator/= (Vec2f vec) {
		x /= vec.x;
		y /= vec.y;
	}


	bool operator== (Vec2f vec) {
		return x == vec.x && y == vec.y;
	}

	float x = 0;
	float y = 0;
};
