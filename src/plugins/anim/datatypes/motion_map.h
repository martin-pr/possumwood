#pragma once

#include "animation.h"
#include "metric.h"
#include "filter.h"

namespace anim {

class MotionMap {
	public:
		MotionMap();
		MotionMap(const anim::Animation& a, const ::anim::metric::Base& metric);
		MotionMap(const anim::Animation& ax, const anim::Animation& ay, const ::anim::metric::Base& metric);

		std::size_t width() const;
		std::size_t height() const;

		float operator()(std::size_t x, std::size_t y) const;

		float max() const;
		float min() const;

		void filter(filter::Base& filter);

		void computeLocalMinima(std::size_t count);
		const std::vector<std::pair<std::size_t, std::size_t>>& localMinima() const;

		bool operator ==(const MotionMap& mmap) const;
		bool operator !=(const MotionMap& mmap) const;

	protected:
	private:
		std::size_t m_width;
		std::vector<float> m_data;

		float m_min, m_max;

		std::vector<std::pair<std::size_t, std::size_t>> m_minima;
};

std::ostream& operator <<(std::ostream& out, const MotionMap& mmap);

}

namespace possumwood {

template<>
struct Traits<anim::MotionMap> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 0, 0}};
	}
};

}
