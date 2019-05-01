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

void MotionMap::filter(filter::Base& filter) {
	filter.init(*this);

	std::atomic<float> minVal(std::numeric_limits<float>::max());
	std::atomic<float> maxVal(std::numeric_limits<float>::min());

	std::vector<float> data = m_data;

	tbb::parallel_for(std::size_t(0), height(), [&](std::size_t y) {
		for(std::size_t x=0; x<width(); ++x) {
			const std::size_t index = x + y*m_width;

			const float val = filter.eval(*this, x, y);

			data[index] = val;

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

	m_data = data;

	m_min = minVal;
	m_max = maxVal;
}

namespace {

struct InvertedComparator {
	bool operator()(float a, float b) const {
		return b <= a;
	}
};

}

std::vector<std::pair<std::size_t, std::size_t>> MotionMap::localMinima(std::size_t count) const {
	std::vector<std::pair<std::size_t, std::size_t>> result;

	if(width() >= 3 && height() >= 3) {
		// collect the minima in an ordered container
		std::map<float, std::pair<std::size_t, std::size_t>, InvertedComparator> minima;

		// for each pixel outside the boundary
		for(std::size_t y=1; y<height()-1; ++y)
			for(std::size_t x=1; x<width()-1; ++x) {
				// test all surrounding pixels, to determine if the current pixel is a local minimum
				bool minimum = true;
				for(int a=0;a<9;++a)
					if(a != 4)
						minimum &= (*this)(x, y) <= (*this)((x + (a%3))-1, (y + (a/3))-1);

				// insert the minimum into the sorted container, and keep only 'count' number of elements
				if(minimum) {
					minima.insert(std::make_pair((*this)(x, y), std::make_pair(x, y)));

					if(minima.size() > count)
						minima.erase(minima.begin());
				}
			}

		// convert the sorted container into a vector for return
		for(auto& i : minima)
			result.push_back(i.second);
	}

	return result;
}

}
