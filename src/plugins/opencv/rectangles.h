#pragma once

#include <memory>
#include <vector>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

namespace possumwood {

template<>
struct Traits<std::vector<cv::Rect>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 0.5}};
	}
};

}

namespace cv {

std::ostream& operator << (std::ostream& out, const cv::Rect& rect);

}

namespace std { /// hack! saving myself from writing a wrapper

std::ostream& operator << (std::ostream& out, const std::vector<cv::Rect>& rect);

}