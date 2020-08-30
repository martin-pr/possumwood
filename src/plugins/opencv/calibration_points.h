#pragma once

#include <actions/traits.h>

#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

namespace possumwood {
namespace opencv {

class CalibrationPoints {
  public:
	class Layer {
	  public:
		void add(const cv::Vec3f& objectPoint, const cv::Vec2f& cameraPoint);

	  private:
		std::vector<cv::Vec3f> m_objectPoints;
		std::vector<cv::Vec2f> m_cameraPoints;

		friend class CalibrationPoints;
	};

	CalibrationPoints(const cv::Size& imageSize = cv::Size());

	void addLayer(const Layer& l);

	bool empty() const;
	std::size_t size() const;

	const std::vector<std::vector<cv::Vec3f>>& objectPoints() const;
	const std::vector<std::vector<cv::Vec2f>>& cameraPoints() const;
	const cv::Size& imageSize() const;

	bool operator==(const CalibrationPoints& f) const;
	bool operator!=(const CalibrationPoints& f) const;

  private:
	std::vector<std::vector<cv::Vec3f>> m_objectPoints;
	std::vector<std::vector<cv::Vec2f>> m_cameraPoints;
	cv::Size m_imageSize;
};

std::ostream& operator<<(std::ostream& out, const CalibrationPoints& f);

}  // namespace opencv

template <>
struct Traits<opencv::CalibrationPoints> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.33, 0.66, 0}};
	}
};

}  // namespace possumwood
