#pragma once

#include <memory>
#include <iostream>

#include <actions/traits.h>

#include <opencv2/opencv.hpp>

namespace possumwood { namespace opencv {

class LightfieldPattern {
	public:
		LightfieldPattern();
		LightfieldPattern(double lensPitch, double pixelPitch, double rotation, 
			cv::Vec2d scaleFactor, cv::Vec3d sensorOffset, cv::Vec2i sensorResolution);

		cv::Vec4f sample(const cv::Vec2i& pixelPos) const;

		bool operator == (const LightfieldPattern& f) const;
		bool operator != (const LightfieldPattern& f) const;

	private:
		double m_lensPitch, m_pixelPitch, m_rotation;
		cv::Vec2d m_scaleFactor;
		cv::Vec3d m_sensorOffset;
		cv::Vec2i m_sensorResolution;
};

std::ostream& operator << (std::ostream& out, const LightfieldPattern& f);

}

template<>
struct Traits<opencv::LightfieldPattern> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0, 0}};
	}
};

}
