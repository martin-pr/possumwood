#pragma once

#include <array>

#include <opencv2/opencv.hpp>

namespace possumwood { namespace opencv {

class BSpline {
	public:
		BSpline(std::size_t subdiv);

		void addSample(const std::array<double, 2>& coords, double value);
		double sample(const std::array<double, 2>& coords) const;

	private:
		std::size_t m_subdiv;
		std::vector<std::pair<float, float>> m_controls;

	friend std::ostream& operator << (std::ostream& out, const BSpline& bs);
};

} }
