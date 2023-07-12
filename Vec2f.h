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

	Vec2f operator+ (Vec2f vec) {
		return Vec2f(x + vec.x, y + vec.y);
	}

	Vec2f operator- (Vec2f vec) {
		return Vec2f(x - vec.x, y - vec.y);
	}

	Vec2f operator* (float num) {
		return Vec2f(x * num, y * num);
	}

	bool operator== (Vec2f vec) {
		return x == vec.x && y == vec.y;
	}

	float x = 0;
	float y = 0;
};
