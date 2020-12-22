#pragma once

#include <dependency_graph/data_traits.h>

#include "actions/traits.h"

#include "opencv/bspline.h"

namespace possumwood {

template <unsigned DEGREE>
struct Traits<possumwood::opencv::BSpline<DEGREE>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 1, 0}};
	}
};

}  // namespace possumwood
