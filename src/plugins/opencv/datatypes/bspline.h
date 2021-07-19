#pragma once

#include <iostream>

#include <dependency_graph/data_traits.h>

#include "actions/traits.h"

#include "opencv/bspline.h"

namespace possumwood {

template <unsigned DEGREE>
struct Traits<std::array<possumwood::opencv::BSpline<DEGREE>, 3>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 1, 0}};
	}
};

namespace opencv {

template <unsigned DEGREE>
std::ostream& operator<<(std::ostream& out, const std::array<possumwood::opencv::BSpline<DEGREE>, 3>&) {
	out << "spline data";
	return out;
}

}  // namespace opencv

}  // namespace possumwood
