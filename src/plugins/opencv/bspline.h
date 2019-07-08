#pragma once

#include <opencv2/opencv.hpp>

namespace possumwood { namespace opencv {

class BSpline {
	public:
		BSpline(std::size_t subdiv);

		void addSample(double x, double y, double value);
		double sample(double x, double y) const;

	private:
		std::size_t m_subdiv;
		cv::Mat m_controls;
		cv::Mat m_norm;

	friend std::ostream& operator << (std::ostream& out, const BSpline& bs);
};

std::ostream& operator << (std::ostream& out, const BSpline& bs);

} }