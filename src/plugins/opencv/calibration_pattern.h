#pragma once

#include <actions/traits.h>

#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

namespace possumwood {
namespace opencv {

class CalibrationPattern {
  public:
	enum Type {
		kSymmetricCirclesGrid,
		kAsymmetricCirclesGrid,
		kChessboard,
	};

	CalibrationPattern(const cv::Mat& data = cv::Mat(), const cv::Size& size = cv::Size(0, 0), bool wasFound = false,
	                   const Type& type = kSymmetricCirclesGrid, const cv::Size& imageSize = cv::Size(0, 0));

	const cv::Mat& operator*() const;
	const cv::Size& size() const;
	const cv::Size& imageSize() const;
	Type type() const;
	bool wasFound() const;

	bool operator==(const CalibrationPattern& f) const;
	bool operator!=(const CalibrationPattern& f) const;

  private:
	std::shared_ptr<const cv::Mat> m_features;  // 2D or 3D list of points
	cv::Size m_size, m_imageSize;
	bool m_wasFound;
	Type m_type;
};

std::ostream& operator<<(std::ostream& out, const CalibrationPattern& f);

}  // namespace opencv

template <>
struct Traits<opencv::CalibrationPattern> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.3, 0.3, 0}};
	}
};

}  // namespace possumwood
