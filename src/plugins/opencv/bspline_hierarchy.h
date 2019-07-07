#pragma once

#include "bspline.h"

namespace possumwood { namespace opencv {

class BSplineHierarchy {
	public:
		BSplineHierarchy(std::size_t level_count, std::size_t level_offset);

		BSpline& level(std::size_t level);

		double sample(double x, double y) const;

	private:
		std::vector<BSpline> m_levels;
};

} }