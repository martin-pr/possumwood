#pragma once

#include <actions/traits.h>

#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

#include "bspline.h"
#include "lightfields.h"

namespace possumwood {
namespace opencv {

class LightfieldVignetting {
  public:
	LightfieldVignetting();
	LightfieldVignetting(std::size_t subdiv, const lightfields::Pattern& pattern, const cv::Mat& image);

	float sample(const cv::Vec4f& coord) const;

	bool operator==(const LightfieldVignetting& f) const;
	bool operator!=(const LightfieldVignetting& f) const;

  private:
	BSpline<4> m_bspline;
};

std::ostream& operator<<(std::ostream& out, const LightfieldVignetting& f);

}  // namespace opencv

template <>
struct Traits<opencv::LightfieldVignetting> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.5, 0}};
	}
};

}  // namespace possumwood
