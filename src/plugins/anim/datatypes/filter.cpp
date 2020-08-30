#include "filter.h"

#include "motion_map.h"

namespace anim {
namespace filter {

Base::~Base() {
}

void Base::init(const MotionMap& mmap) {
}

///////

LinearTransition::LinearTransition(const std::size_t transitionLength)
    : m_halfWindowWidth(transitionLength / 2 + transitionLength % 2) {
}

float LinearTransition::eval(const MotionMap& mmap, std::size_t x, std::size_t y) const {
	if((int)x < m_halfWindowWidth || (int)x >= (int)mmap.width() - m_halfWindowWidth || (int)y < m_halfWindowWidth ||
	   (int)y >= (int)mmap.height() - m_halfWindowWidth)
		return mmap.max();

	float result = 0.0f;
	for(int a = -m_halfWindowWidth; a <= m_halfWindowWidth; ++a)
		result += mmap((int)x + a, (int)y + a);
	result /= 2 * m_halfWindowWidth + 1;

	return result;
}

////////

IgnoreIdentity::IgnoreIdentity(const std::size_t transitionLength)
    : m_halfWindowWidth(transitionLength / 2 + transitionLength % 2) {
}

float IgnoreIdentity::eval(const MotionMap& mmap, std::size_t x, std::size_t y) const {
	if(x >= y && x - y <= m_halfWindowWidth)
		return mmap.max();

	if(x <= y && y - x <= m_halfWindowWidth)
		return mmap.max();

	return mmap(x, y);
}

}  // namespace filter
}  // namespace anim
