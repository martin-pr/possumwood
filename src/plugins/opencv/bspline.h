#pragma once

#include <array>

#include <opencv2/opencv.hpp>

namespace possumwood { namespace opencv {

template<unsigned DEGREE>
class BSpline {
	public:
		BSpline(std::size_t subdiv);

		void addSample(const std::array<double, DEGREE>& coords, double value);
		double sample(const std::array<double, DEGREE>& coords) const;

	private:
		static double B(double t, unsigned k);

		std::size_t m_subdiv;
		std::vector<std::pair<float, float>> m_controls;
};

} }
