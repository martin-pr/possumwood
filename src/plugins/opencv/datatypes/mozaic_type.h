#pragma once

#include <dependency_graph/data_traits.h>

#include "actions/io.h"
#include "actions/traits.h"

namespace possumwood {

// the values here correspond to the offsets in OpenCV's demozaicing enums
// TODO: these are a bit mysterious - they don't directly match with MozaicType from lightfields::Bayer.
// The current version just tries to look "right" for the Lytro input, thats all.
enum class MozaicType { BG = 2, GB = 3, RG = 0, GR = 1 };

template <>
struct Traits<MozaicType> {
	static IO<MozaicType> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0, 0.5}};
	}
};

std::ostream& operator<<(std::ostream& out, const MozaicType& f);

}  // namespace possumwood
