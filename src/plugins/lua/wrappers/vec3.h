#pragma once

#include <iostream>

#include <OpenEXR/ImathVec.h>

namespace possumwood {

namespace lua {

struct Vec3 {
  public:
	Vec3(const Imath::V3f& val = Imath::V3f(0, 0, 0)) : x(val.x), y(val.y), z(val.z) {
	}

	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {
	}

	operator Imath::V3f() const;

	bool operator==(const Vec3& v) const {
		return x == v.x && y == v.y && z == v.z;
	}

	bool operator!=(const Vec3& v) const {
		return x != v.x || y != v.y || z != v.z;
	}

	Vec3 operator+(const Vec3& other) const {
		return Vec3(x + other.x, y + other.y, z + other.z);
	}

	Vec3 operator-(const Vec3& other) const {
		return Vec3(x - other.x, y - other.y, z - other.z);
	}

	Vec3 operator*(float other) const {
		return Vec3(x * other, y * other, z * other);
	}

	float x, y, z;
};

inline Vec3 operator*(float other, const Vec3& val) {
	return Vec3(val.x * other, val.y * other, val.z * other);
}

inline std::string to_string(const Vec3& wrapper) {
	std::stringstream out;
	out << wrapper.x << ", " << wrapper.y << ", " << wrapper.z;
	return out.str();
}

}  // namespace lua

}  // namespace possumwood
