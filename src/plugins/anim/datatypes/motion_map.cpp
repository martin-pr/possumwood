#include "motion_map.h"

#include <tbb/parallel_for.h>

namespace anim {

MotionMap::MotionMap() : m_width(0), m_min(0.0f), m_max(0.0f) {
}

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

void MotionMap::computeLocalMinima(std::size_t count) {
	m_minima.clear();

	if(width() >= 3 && height() >= 3) {
		// collect the minima in an ordered container
		std::multimap<float, std::pair<std::size_t, std::size_t>> minima;

		// for each pixel outside the boundary
		for(std::size_t y=0; y<height()-2; ++y)
			for(std::size_t x=0; x<width()-2; ++x) {
				// test all surrounding pixels, to determine if the current pixel is a local minimum
				bool minimum = true;
				for(int a=0;a<9;++a)
					if(a != 4)
						minimum &= (*this)(x+1, y+1) <= (*this)((x + (a%3)), (y + (a/3)));

				// insert the minimum into the sorted container
				if(minimum)
					minima.insert(std::make_pair((*this)(x+1, y+1), std::make_pair(x+1, y+1)));
			}

		// convert the sorted container into a vector for return
		auto it = minima.begin();
		while(it != minima.end() && m_minima.size() < count) {
			m_minima.push_back(it->second);
			++it;
		}
	}
}

const std::vector<std::pair<std::size_t, std::size_t>>& MotionMap::localMinima() const {
	return m_minima;
}

bool MotionMap::operator ==(const MotionMap& mmap) const {
	return m_min == mmap.m_min && m_max == mmap.m_max && m_data == mmap.m_data;
}

bool MotionMap::operator !=(const MotionMap& mmap) const {
	return m_min != mmap.m_min || m_max != mmap.m_max || m_data != mmap.m_data;
}

std::ostream& operator <<(std::ostream& out, const MotionMap& mmap) {
	out << "(motion map " << mmap.width() << "x" << mmap.height() << ")";
	return out;
}

}
