#include "bspline.h"

#include <opencv2/opencv.hpp>

namespace possumwood { namespace opencv {

template<unsigned DEGREE>
double BSpline<DEGREE>::B(double t, unsigned k) {
	assert(t >= 0.0 && t <= 1.0);
	assert(k < 4);

	if(k == 0)
		return std::pow(1.0-t, 3) / 6.0;
	else if(k == 1)
		return (3.0*std::pow(t, 3) - 6.0*t*t + 4.0) / 6.0;
	else if(k == 2)
		return (-3.0*std::pow(t, 3) + 3.0*t*t + 3.0*t + 1) / 6.0;
	else
		return std::pow(t, 3) / 6.0;
}

template<unsigned DEGREE>
std::array<double, DEGREE> BSpline<DEGREE>::initArray(double val) {
	std::array<double, DEGREE> result;
	result.fill(val);
	return result;
}

template<unsigned DEGREE>
BSpline<DEGREE>::BSpline(unsigned subdiv, const std::array<double, DEGREE>& min, const std::array<double, DEGREE>& max) : m_subdiv(subdiv), m_controls((subdiv+3) * (subdiv+3), std::make_pair(0.0f, 0.0f)), m_min(min), m_max(max) {
	assert(subdiv > 0);
}

template<unsigned DEGREE>
void BSpline<DEGREE>::visit(const std::array<double, DEGREE>& _coords, std::function<void(unsigned, float)> fn) const {
	std::array<double, DEGREE> coords = _coords;
	std::array<unsigned, DEGREE> offset;

	for(unsigned d=0; d<coords.size(); ++d) {
		coords[d] = (coords[d] - m_min[d]) / (m_max[d] - m_min[d]);

		assert(coords[d] >= 0.0 && coords[d] <= 1.0);

		coords[d] *= (double)m_subdiv;

		offset[d] = floor(coords[d]);
		coords[d] = fmod(coords[d], 1.0);
	}

	for(unsigned i=0;i<pow(4, coords.size()); ++i) {
		double weight = 1.0f;
		unsigned j = i;
		std::size_t index = 0;

		for(unsigned d=0; d<coords.size(); ++d) {
			weight *= B(coords[d], j % 4);
			index = index * m_subdiv + j%4 + offset[d];

			j /= 4;
		}

		fn(index, weight);
	}
}

template<unsigned DEGREE>
void BSpline<DEGREE>::addSample(const std::array<double, DEGREE>& coords, double value) {
	visit(coords, [&](unsigned index, double weight) {
		m_controls[index].first += weight * value;
		m_controls[index].second += weight;
	});
}

template<unsigned DEGREE>
double BSpline<DEGREE>::sample(const std::array<double, DEGREE>& coords) const {
	double result = 0;
	visit(coords, [&](unsigned index, double weight) {
		result += m_controls[index].first / m_controls[index].second * weight;
	});

	return result;
}

} }
