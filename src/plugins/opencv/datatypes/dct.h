#pragma once

#include <dependency_graph/data_traits.h>

#include "actions/io.h"
#include "actions/traits.h"

#include "lightfields/dct.h"

namespace possumwood {

template <>
struct Traits<lightfields::DCT> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 1, 0}};
	}
};

}  // namespace possumwood
