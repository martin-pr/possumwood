#pragma once

#include <OpenEXR/ImathVec.h>
#include <actions/traits.h>

namespace possumwood {

template <>
struct Traits<Imath::Vec3<float>> {
	static IO<Imath::Vec3<float>> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 1}};
	}
};

template <>
struct Traits<Imath::Vec3<unsigned>> {
	static IO<Imath::Vec3<unsigned>> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.8, 1}};
	}
};

template <>
struct Traits<Imath::Vec3<int>> {
	static IO<Imath::Vec3<int>> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.6, 1}};
	}
};

}  // namespace possumwood
