#pragma once

#include <actions/traits.h>

#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

namespace possumwood {
namespace opencv {

class CameraIntrinsics {
  public:
	CameraIntrinsics();
	CameraIntrinsics(const cv::Mat& matrix, const std::vector<float>& distCoeffs, const cv::Size& imageSize);

	const cv::Mat& matrix() const;
	const std::vector<float>& distCoeffs() const;
	const cv::Size& imageSize() const;

	bool operator==(const CameraIntrinsics& f) const;
	bool operator!=(const CameraIntrinsics& f) const;

  private:
	cv::Mat m_matrix;
	std::vector<float> m_distCoeffs;
	cv::Size m_imageSize;
};

std::ostream& operator<<(std::ostream& out, const CameraIntrinsics& f);

}  // namespace opencv

template <>
struct Traits<opencv::CameraIntrinsics> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.3, 1, 0}};
	}
};

}  // namespace possumwood
