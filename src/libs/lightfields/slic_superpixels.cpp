#include "slic_superpixels.h"

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

int SlicSuperpixels::initS(int rows, int cols, int pixelCount) {
	int result = std::sqrt((cols * rows) / pixelCount);
	if(result <= 0)
		throw std::runtime_error("Only positive pixel counts are allowed.");

	return result;
}

}
