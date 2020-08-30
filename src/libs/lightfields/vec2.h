#pragma once

namespace lightfields {

template <typename T>
struct Vec2 {
	Vec2() : x(0), y(0) {
	}

	Vec2(const T& _x, const T& _y) : x(_x), y(_y) {
	}

	bool operator==(const Vec2& v) const {
		return x == v.x && y == v.y;
	}

	bool operator!=(const Vec2& v) const {
		return x != v.x || y != v.y;
	}

	static int sqdist(const Vec2& v1, const Vec2& v2) {
		return (v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y);
	}

	Vec2 operator-(const Vec2& other) const {
		return Vec2(x - other.x, y - other.y);
	}

	Vec2 operator+(const Vec2& other) const {
		return Vec2(x + other.x, y + other.y);
	}

	T x, y;
};

typedef Vec2<int> V2i;
typedef Vec2<float> V2f;

}  // namespace lightfields
