#include "slic_superpixels.h"

#include <tbb/parallel_for.h>

namespace lightfields {

SlicSuperpixels::Center::Center() : row(0), col(0), color{{0, 0, 0}} {
}

SlicSuperpixels::Center::Center(const cv::Mat& m, int r, int c) : row(r), col(c) {
	assert(r >= 0 && r < m.rows);
	assert(c >= 0 && c < m.cols);

	auto ptr = m.ptr<unsigned char>(r, c);
	for(int a=0;a<3;++a)
		color[a] = ptr[a];
}

SlicSuperpixels::Center& SlicSuperpixels::Center::operator +=(const Center& c) {
	row += c.row;
	col += c.col;

	for(int a=0;a<3;++a)
		color[a] += c.color[a];

	return *this;
}

SlicSuperpixels::Center& SlicSuperpixels::Center::operator /=(int div) {
	row /= div;
	col /= div;

	for(int a=0;a<3;++a)
		color[a] /= div;

	return *this;
}

////////////

SlicSuperpixels::Metric::Metric(int S, float m) : m_S(S), m_SS(S*S), m_mm(m*m) {
}

float SlicSuperpixels::Metric::operator()(const lightfields::SlicSuperpixels::Center& c, const cv::Mat& m, const int row, const int col) const {
	float d_c = 0.0f;
	for(int a=0;a<3;++a) {
		float elem = float(c.color[a]) - float(m.ptr<unsigned char>(row, col)[a]);
		elem *= elem;

		d_c += elem;
	}

	const float d_s = float(c.row - row)*float(c.row - row) + float(c.col - col)*float(c.col - col);

	return std::sqrt(d_c + d_s / m_SS * m_mm);
}

int SlicSuperpixels::Metric::S() const {
	return m_S;
}

////////////

int SlicSuperpixels::initS(int rows, int cols, int pixelCount) {
	int result = std::sqrt((cols * rows) / pixelCount);
	if(result <= 0)
		throw std::runtime_error("Only positive pixel counts are allowed.");

	return result;
}

lightfields::Grid<lightfields::SlicSuperpixels::Center> SlicSuperpixels::initPixels(const cv::Mat& in, int S) {
	const int rows = in.rows / S;
	const int cols = in.cols / S;

	lightfields::Grid<lightfields::SlicSuperpixels::Center> pixels(rows, cols);

	for(int y=0; y<rows; ++y)
		for(int x=0; x<cols; ++x)
			pixels(y, x) = lightfields::SlicSuperpixels::Center(in, (in.rows * y) / rows + S/2, (in.cols * x) / cols + S/2);

	return pixels;
}

void SlicSuperpixels::label(const cv::Mat& in, lightfields::Grid<std::atomic<Label>>& labels, const lightfields::Grid<lightfields::SlicSuperpixels::Center>& centers, const Metric& metric) {
	assert(in.rows == (int)labels.rows() && in.cols == (int)labels.cols());

	// using the metric instance, label all pixels
	tbb::parallel_for(0, int(centers.container().size()), [&](int a) {
		const lightfields::SlicSuperpixels::Center& center = centers.container()[a];

		for(int row = std::max(0, center.row-metric.S()); row < std::min(in.rows, center.row+metric.S()+1); ++row)
			for(int col = std::max(0, center.col-metric.S()); col < std::min(in.cols, center.col+metric.S()+1); ++col) {
				const Label next(a, metric(center, in, row, col));
				std::atomic<Label>& current = labels(row, col);

				Label tmp(0, 0);
				do {
					tmp = current;
				} while(tmp.metric > next.metric && !current.compare_exchange_weak(tmp, next));
			}
	});
}

}
