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

	float x, y, z;
};

inline std::string to_string(const Vec3& wrapper) {
	std::stringstream out;
	out << wrapper.x << ", " << wrapper.y << ", " << wrapper.z;
	return out.str();
}

}  // namespace lua

}  // namespace possumwood
