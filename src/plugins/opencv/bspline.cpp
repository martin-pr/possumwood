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

BSpline::BSpline(std::size_t subdiv) : m_subdiv(subdiv), m_controls(cv::Mat::zeros(subdiv+3, subdiv+3, CV_32FC1)), m_norm(cv::Mat::zeros(subdiv+3, subdiv+3, CV_32FC1)) {
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
			const double weight = B(x, a) * B(y, b);
			m_controls.at<float>(b+offs_y, a+offs_x) += weight * value;
			m_norm.at<float>(b+offs_y, a+offs_x) += weight;
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
			result += m_controls.at<float>(b+offs_y, a+offs_x) / 
				m_norm.at<float>(b+offs_y, a+offs_x) * weight;
		}

	return result;
}

std::ostream& operator << (std::ostream& out, const BSpline& bs) {
	for(int a=0;a<bs.m_controls.rows;++a) {
		for(int b=0;b<bs.m_controls.cols;++b) {
			out << (bs.m_controls.at<float>(a,b) / bs.m_norm.at<float>(a,b)) << "  ";
		}

		out << std::endl;
	}
	out << std::endl;

	return out;
}

} }