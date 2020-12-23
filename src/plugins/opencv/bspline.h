#pragma once

#include <array>
#include <functional>
#include <opencv2/opencv.hpp>

namespace possumwood {
namespace opencv {

template <unsigned DEGREE>
class BSpline {
  public:
	BSpline(unsigned subdiv = 0,
	        const std::array<float, DEGREE>& min = initArray(0.0),
	        const std::array<float, DEGREE>& max = initArray(1.0));

	void addSample(const std::array<float, DEGREE>& coords, float value);
	float sample(const std::array<float, DEGREE>& coords) const;

	bool operator==(const BSpline& b) const;
	bool operator!=(const BSpline& b) const;

  private:
	static float B(float t, unsigned k);

	template <typename FN>
	inline void visit(const std::array<float, DEGREE>& coords, const FN& fn) const;
	static std::array<float, DEGREE> initArray(float val);

	std::size_t m_subdiv;
	std::vector<std::pair<float, float>> m_controls;
	std::array<float, DEGREE> m_min, m_max;
};

template <unsigned DEGREE>
std::ostream& operator<<(std::ostream& out, const BSpline<DEGREE>& spline);

}  // namespace opencv
}  // namespace possumwood
