#include "lightfield_vignetting.h"

#include <tbb/parallel_for.h>

#include "bspline.inl"

namespace possumwood { namespace opencv {

LightfieldVignetting::LightfieldVignetting() : m_bspline(1, {{0, 0, -1, -1}}, {{1, 1, 1, 1}}) {
	m_bspline.addSample({{0.5, 0.5, 0, 0}}, 1.0f);
}

LightfieldVignetting::LightfieldVignetting(std::size_t subdiv, const lightfields::Pattern& pattern, const cv::Mat& image) :

m_bspline(subdiv, {{0, 0, -1, -1}}, {{1, 1, 1, 1}}) {
	if(image.rows != pattern.sensorResolution()[1] || image.cols != pattern.sensorResolution()[0])
		throw std::runtime_error("Pattern and image resolution doesn't match!");

	tbb::parallel_for(0, image.rows, [&](int y) {
		for(int x=0;x<image.cols;++x) {
			const Imath::V4d& coord = pattern.sample(Imath::V2i{x, y});

			const double uv_magnitude_2 = coord[2]*coord[2] + coord[3]*coord[3];
			if(uv_magnitude_2 < 1.0) {
				const double xf = (double)x / (double)(image.cols-1);
				const double yf = (double)y / (double)(image.rows-1);

				m_bspline.addSample({{
						xf, yf,
						coord[2], coord[3]
					}},
					image.at<float>(y, x));
			}
		}
	});
}

double LightfieldVignetting::sample(const cv::Vec4f& coord) const {
	return m_bspline.sample({{coord[0], coord[1], coord[2], coord[3]}});
}

bool LightfieldVignetting::operator == (const LightfieldVignetting& f) const {
	return m_bspline == f.m_bspline;
}

bool LightfieldVignetting::operator != (const LightfieldVignetting& f) const {
	return m_bspline != f.m_bspline;
}

std::ostream& operator << (std::ostream& out, const LightfieldVignetting& f) {
	out << "(lightfield vigentting data)";
	return out;
}

} }
