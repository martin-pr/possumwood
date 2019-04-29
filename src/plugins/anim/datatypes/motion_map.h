#pragma once

#include "animation.h"
#include "metric.h"
#include "filter.h"

namespace anim {

class MotionMap {
	public:
		MotionMap(const anim::Animation& a, const ::anim::metric::Base& metric);
		MotionMap(const anim::Animation& ax, const anim::Animation& ay, const ::anim::metric::Base& metric);

		std::size_t width() const;
		std::size_t height() const;

		float operator()(std::size_t x, std::size_t y) const;

		float max() const;
		float min() const;

		void filter(filter::Base& filter);

	protected:
	private:
		std::size_t m_width;
		std::vector<float> m_data;

		float m_min, m_max;
};

}
