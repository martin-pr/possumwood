#pragma once

#include <dependency_graph/data_traits.h>

#include "actions/io.h"
#include "actions/traits.h"

namespace possumwood {

// the values here correspond to the offsets in OpenCV's demozaicing enums
enum class MozaicType { BG = 0, GB = 1, RG = 2, GR = 3 };

template <>
struct Traits<MozaicType> {
	static IO<MozaicType> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0, 0.5}};
	}
};

std::ostream& operator<<(std::ostream& out, const MozaicType& f);

}  // namespace possumwood
