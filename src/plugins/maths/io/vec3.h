#pragma once

#include <OpenEXR/ImathVec.h>

#include <actions/traits.h>

namespace possumwood {

template<>
struct Traits<Imath::Vec3<float>> {
	static IO<Imath::Vec3<float>> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 1}};
	}
};

}
