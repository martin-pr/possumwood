#pragma once

#include <actions/traits.h>

#include <memory>
#include <opencv2/opencv.hpp>
#include <vector>

namespace possumwood {

template <>
struct Traits<std::vector<cv::Rect>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 0.5}};
	}
};

}  // namespace possumwood

namespace cv {

std::ostream& operator<<(std::ostream& out, const cv::Rect& rect);

}

namespace std {  /// hack! saving myself from writing a wrapper

std::ostream& operator<<(std::ostream& out, const std::vector<cv::Rect>& rect);

}