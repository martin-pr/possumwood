#pragma once

#include <actions/traits.h>

#include <memory>
#include <opencv2/opencv.hpp>
#include <vector>

namespace possumwood {
namespace opencv {

class Frame {
  public:
	Frame(const cv::Mat& data = cv::Mat(), bool copy = true);

	Frame clone() const;

	const cv::Mat& operator*() const;
	const cv::Mat* operator->() const;

	cv::Mat& operator*();
	cv::Mat* operator->();

	cv::Size size() const;
	bool empty() const;

	bool operator==(const Frame& f) const;
	bool operator!=(const Frame& f) const;

  private:
	cv::Mat m_frame;
};

std::ostream& operator<<(std::ostream& out, const Frame& f);

}  // namespace opencv

template <>
struct Traits<opencv::Frame> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 0}};
	}
};

}  // namespace possumwood
