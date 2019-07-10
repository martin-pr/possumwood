#include "bspline.h"

#include <opencv2/opencv.hpp>

namespace possumwood { namespace opencv {

namespace {

double B0(double t) {
	return std::pow(1.0-t, 3) / 6.0;
}

double B1(double t) {
	return (3.0*std::pow(t, 3) - 6.0*t*t + 4.0) / 6.0;
}

double B2(double t) {
	return (-3.0*std::pow(t, 3) + 3.0*t*t + 3.0*t + 1) / 6.0;
}

double B3(double t) {
	return std::pow(t, 3) / 6.0;
}

double B(double t, unsigned k) {
	assert(t >= 0.0 && t <= 1.0);
	assert(k < 4);

	if(k == 0)
		return B0(t);
	else if(k == 1)
		return B1(t);
	else if(k == 2)
		return B2(t);
	else
		return B3(t);
}

}

BSpline::BSpline(std::size_t subdiv) : m_subdiv(subdiv), m_controls((subdiv+3) * (subdiv+3), std::make_pair(0.0f, 0.0f)) {
	assert(subdiv > 0);
}

void BSpline::addSample(double x, double y, double value) {
	assert(x >= 0.0 && x <= 1.0);
	assert(y >= 0.0 && y <= 1.0);

	x = x * (double)m_subdiv;
	y = y * (double)m_subdiv;

	const std::size_t offs_x = floor(x);
	const std::size_t offs_y = floor(y);

	x = fmod(x, 1.0);
	y = fmod(y, 1.0);

	for(unsigned a=0;a<4;++a)
		for(unsigned b=0;b<4;++b) {
			const double weight = B(x, a) * B(y, b)*m_subdiv;

			const std::size_t index = (a+offs_x) + (b+offs_y)*m_subdiv;

			m_controls[index].first += weight * value;
			m_controls[index].second += weight;
		}
}

double BSpline::sample(double x, double y) const {
	assert(x >= 0.0 && x <= 1.0);
	assert(y >= 0.0 && y <= 1.0);

	x = x * (double)m_subdiv;
	y = y * (double)m_subdiv;

	const std::size_t offs_x = floor(x);
	const std::size_t offs_y = floor(y);

	x = fmod(x, 1.0);
	y = fmod(y, 1.0);

	double result = 0;

	for(unsigned a=0;a<4;++a)
		for(unsigned b=0;b<4;++b) {
			const double weight = B(x, a) * B(y, b);

			const std::size_t index = (a+offs_x) + (b+offs_y)*m_subdiv;

			result += m_controls[index].first / m_controls[index].second * weight;
		}

	return result;
}

} }
