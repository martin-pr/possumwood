#pragma once

#include <memory>
#include <iostream>

#include <actions/traits.h>

#include <opencv2/opencv.hpp>

#include "lightfield_pattern.h"
#include "bspline.h"

namespace possumwood { namespace opencv {

class LightfieldVignetting {
	public:
		LightfieldVignetting();
		LightfieldVignetting(std::size_t subdiv, const LightfieldPattern& pattern, const cv::Mat& image);

		double sample(const cv::Vec4f& coord) const;

		bool operator == (const LightfieldVignetting& f) const;
		bool operator != (const LightfieldVignetting& f) const;

	private:
		BSpline<4> m_bspline;
};

std::ostream& operator << (std::ostream& out, const LightfieldVignetting& f);

}

template<>
struct Traits<opencv::LightfieldVignetting> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.5, 0}};
	}
};

}
