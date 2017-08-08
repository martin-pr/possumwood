#pragma once

#include <OpenEXR/ImathVec.h>

#include <possumwood_sdk/traits.h>

namespace possumwood {

template<>
struct Traits<Imath::Vec3<float>> {
	static IO<Imath::Vec3<float>> io;
};

}
