#include "motion_map.h"

#include <tbb/parallel_for.h>

namespace anim {

MotionMap::MotionMap(const anim::Animation& a, const ::anim::metric::Base& metric) : m_width(a.size()), m_data(a.size()*a.size()), m_min(std::numeric_limits<float>::max()), m_max(std::numeric_limits<float>::min()) {
	std::atomic<float> minVal(m_min);
	std::atomic<float> maxVal(m_max);

	tbb::parallel_for(std::size_t(0), a.size(), [&](std::size_t y) {
		for(std::size_t x=y; x<a.size(); ++x) {
			const std::size_t index1 = x + y*m_width;
			const std::size_t index2 = y + x*m_width;

			const float val = metric.eval(a, x, a, y);

			m_data[index1] = val;
			m_data[index2] = val;

			{
				float tmp = val;
				while(tmp < minVal)
					tmp = minVal.exchange(tmp);
			}

			{
				float tmp = val;
				while(tmp > maxVal)
					tmp = maxVal.exchange(tmp);
			}
		}
	});

	m_min = minVal;
	m_max = maxVal;
}

MotionMap::MotionMap(const anim::Animation& ax, const anim::Animation& ay, const ::anim::metric::Base& metric) : m_width(ax.size()), m_data(ax.size()*ay.size()), m_min(std::numeric_limits<float>::max()), m_max(std::numeric_limits<float>::min()) {
	std::atomic<float> minVal(m_min);
	std::atomic<float> maxVal(m_max);

	tbb::parallel_for(std::size_t(0), ay.size(), [&](std::size_t y) {
		for(std::size_t x=0; x<ax.size(); ++x) {
			const std::size_t index = x + y*m_width;

			const float val = metric.eval(ax, x, ay, y);

			m_data[index] = val;

			{
				float tmp = val;
				while(tmp < minVal)
					tmp = minVal.exchange(tmp);
			}

			{
				float tmp = val;
				while(tmp > maxVal)
					tmp = maxVal.exchange(tmp);
			}
		}
	});

	m_min = minVal;
	m_max = maxVal;
}

std::size_t MotionMap::width() const {
	return m_width;
}

std::size_t MotionMap::height() const {
	if(m_data.empty())
		return 0;
	return m_data.size() / m_width;
}

float MotionMap::operator()(std::size_t x, std::size_t y) const {
	assert(x < m_width);
	assert(y < height());

	return m_data[x + y*m_width];
}

float MotionMap::max() const {
	return m_max;
}

float MotionMap::min() const {
	return m_min;
}


}
