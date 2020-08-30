#pragma once

#include <array>
#include <functional>
#include <opencv2/opencv.hpp>

namespace possumwood {
namespace opencv {

template <unsigned DEGREE>
class BSpline {
  public:
	BSpline(unsigned subdiv, const std::array<double, DEGREE>& min = initArray(0.0),
	        const std::array<double, DEGREE>& max = initArray(1.0));

	void addSample(const std::array<double, DEGREE>& coords, double value);
	double sample(const std::array<double, DEGREE>& coords) const;

	bool operator==(const BSpline& b) const;
	bool operator!=(const BSpline& b) const;

  private:
	static double B(double t, unsigned k);

	template <typename FN>
	inline void visit(const std::array<double, DEGREE>& coords, const FN& fn) const;
	static std::array<double, DEGREE> initArray(double val);

	std::size_t m_subdiv;
	std::vector<std::pair<float, float>> m_controls;
	std::array<double, DEGREE> m_min, m_max;
};

}  // namespace opencv
}  // namespace possumwood
