#include "bspline_hierarchy.h"

#include "bspline.inl"

namespace possumwood {
namespace opencv {

BSplineHierarchy::BSplineHierarchy(std::size_t level_count, std::size_t level_offset) {
	std::size_t level = pow(2, level_offset);
	for(std::size_t i = 0; i < level_count; ++i) {
		m_levels.push_back(BSpline<2>(std::array<std::size_t, 2>{{level, level}}));
		level *= 2;
	}
}

BSpline<2>& BSplineHierarchy::level(std::size_t level) {
	assert(level < m_levels.size());
	return m_levels[level];
}

float BSplineHierarchy::sample(float x, float y) const {
	float result = 0;
	for(auto& l : m_levels)
		result += l.sample({x, y});
	return result;
}

}  // namespace opencv
}  // namespace possumwood
