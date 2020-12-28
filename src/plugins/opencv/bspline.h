#pragma once

#include <array>
#include <functional>
#include <opencv2/opencv.hpp>

namespace possumwood {
namespace opencv {

template <unsigned DEGREE>
class BSpline {
  public:
	explicit BSpline(const std::array<std::size_t, DEGREE>& subdiv = initArray(std::size_t(0)));

	void addSample(const std::array<float, DEGREE>& coords, float value);
	float sample(const std::array<float, DEGREE>& coords) const;

	std::size_t size(unsigned dim) const;

	bool operator==(const BSpline& b) const;
	bool operator!=(const BSpline& b) const;

  private:
	static float B(float t, unsigned k);

	template <typename FN>
	inline void visit(const std::array<float, DEGREE>& coords, const FN& fn) const;

	template <typename T>
	static std::array<T, DEGREE> initArray(T val);

	std::array<std::size_t, DEGREE> m_subdiv;
	std::vector<std::pair<float, float>> m_controls;
};

template <unsigned DEGREE>
std::ostream& operator<<(std::ostream& out, const BSpline<DEGREE>& spline);

}  // namespace opencv
}  // namespace possumwood
