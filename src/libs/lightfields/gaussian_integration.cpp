#include "gaussian_integration.h"

#include <tbb/parallel_for.h>

#include "samples.h"

namespace lightfields { namespace gaussian {

namespace {

void add1channel(float* color, float* n, int colorIndex, const float* value, float gauss) {
	color[colorIndex] += value[0] * gauss;
	n[colorIndex] += gauss;
}

void add3channel(float* color, float* n, int colorIndex, const float* value, float gauss) {
	color[0] += value[0] * gauss;
	n[0] += gauss;

	color[1] += value[1] * gauss;
	n[1] += gauss;

	color[2] += value[2] * gauss;
	n[2] += gauss;
}

}

IntegrationResult integrate(const lightfields::Samples& samples, const Imath::Vec2<unsigned>& size, const cv::Mat& data, float _sigma, float offset) {
	// TODO: for parallelization to work reliably, we need to use integer atomics here, unfortunately

	// SOMEHOW, THE RESULT OF DETAIL IS NOT SMOOTH, AND IT DEFINITELY SHOULD BE
	// CONFIDENCE HAS WEIRD PEAKS, FIX ME!

	if(data.type() != CV_32FC3 && data.type() != CV_32FC1)
		throw std::runtime_error("Nearest neighbour integration - only 1 or 3 channel float data allowed on input.");

	auto applyFn = &add1channel;
	if(data.type() == CV_32FC3)
		applyFn = &add3channel;

	const unsigned width = size[0];
	const unsigned height = size[1];

	cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(height, width, CV_32FC3);

	const float sigma = _sigma * (float)width / (float)data.cols;
	const float sigma2 = 2.0f*sigma*sigma;

	const float x_scale = (float)width / (float)samples.sensorSize()[0];
	const float y_scale = (float)height / (float)samples.sensorSize()[1];

	tbb::parallel_for(0, data.rows, [&](int y) {
		const auto end = samples.end(y);
		for(auto it = samples.begin(y); it != end; ++it) {
			const float target_x = (it->xy[0] + offset * it->uv[0]) * x_scale;
			const float target_y = (it->xy[1] + offset * it->uv[1]) * y_scale;

			int xFrom = std::max((int)floor(target_x - 3.0f*sigma), 0);
			int xTo = std::min((int)ceil(target_x + 3.0f*sigma + 1.0f), (int)width);
			int yFrom = std::max((int)floor(target_y - 3.0f*sigma), 0);
			int yTo = std::min((int)ceil(target_y + 3.0f*sigma + 1.0f), (int)height);

			for(int yt=yFrom; yt<yTo; ++yt) {
				for(int xt=xFrom; xt<xTo; ++xt) {
					const float xf = (float)xt - target_x; // because pow() is expensive?
					const float yf = (float)yt - target_y;

					const float dist2 = xf*xf + yf*yf;
					const float gauss = std::exp(-dist2/sigma2);

					float* color = mat.ptr<float>(yt, xt);
					float* n = norm.ptr<float>(yt, xt);

					const float* value = data.ptr<float>(it->source[1], it->source[0]);
					(*applyFn)(color, n, it->color, value, gauss);
				}
			}
		}
	});

	tbb::parallel_for(0, mat.rows, [&](int y) {
		for(int x=0;x<mat.cols;++x)
			for(int a=0;a<3;++a)
				if(norm.ptr<float>(y,x)[a] > 0.0f)
					mat.ptr<float>(y,x)[a] /= norm.ptr<float>(y,x)[a];
	});

	return IntegrationResult { mat, norm };
}

cv::Mat correspondence(const lightfields::Samples& samples, const cv::Mat& data, const IntegrationResult& integration, float _sigma, float offset) {
	if(data.type() != CV_32FC1 && data.type() != CV_32FC3)
		throw std::runtime_error("Only 32-bit single-float or 32-bit 3 channel float format supported on input.");

	const unsigned width = integration.average.cols;
	const unsigned height = integration.average.rows;

	cv::Mat corresp = cv::Mat::zeros(height, width, CV_32FC1);
	cv::Mat weights = cv::Mat::zeros(height, width, CV_32FC1);

	const float sigma = _sigma * (float)width / (float)data.cols;
	const float sigma2 = 2.0f*sigma*sigma;

	const float x_scale = (float)width / (float)samples.sensorSize()[0];
	const float y_scale = (float)height / (float)samples.sensorSize()[1];

	tbb::parallel_for(0, data.rows, [&](int y) {
		const auto end = samples.end(y);
		const auto begin = samples.begin(y);
		assert(begin <= end);

		for(auto it = begin; it != end; ++it) {
			const float target_x = (it->xy[0] + offset * it->uv[0]) * x_scale;
			const float target_y = (it->xy[1] + offset * it->uv[1]) * y_scale;

			int xFrom = std::max((int)floor(target_x - 3.0f*sigma), 0);
			int xTo = std::min((int)ceil(target_x + 3.0f*sigma + 1.0f), (int)width);
			int yFrom = std::max((int)floor(target_y - 3.0f*sigma), 0);
			int yTo = std::min((int)ceil(target_y + 3.0f*sigma + 1.0f), (int)height);

			for(int yt=yFrom; yt<yTo; ++yt) {
				for(int xt=xFrom; xt<xTo; ++xt) {
					const float xf = (float)xt - target_x; // because pow() is expensive?
					const float yf = (float)yt - target_y;

					const float dist2 = xf*xf + yf*yf;
					const float gauss = std::exp(-dist2/sigma2);

					float* target = corresp.ptr<float>(yt, xt);
					float* weight = weights.ptr<float>(yt, xt);

					const float* value = data.ptr<float>(it->source[1], it->source[0]);
					const float* ave = integration.average.ptr<float>(yt, xt);

					// weighted variance of the samples.
					// The algorithm to do this is a bit dubious - acc to wikipedia and stack overflow, weighted variance computation
					// is much more complex than this simple weighting. This might be the reason for the "random peaks" in the confidence
					// value. Ref: // https://www.itl.nist.gov/div898/software/dataplot/refman2/ch2/weighvar.pdf
					if(data.channels() == 3)
						for(int c=0; c<3; ++c) {
							const float tmp = value[c] - ave[c]; // because pow() is expensive?!
							*target += tmp*tmp * gauss;
							*weight += gauss;
						}
					else {
						const float tmp = *value - ave[it->color];
						*target += tmp*tmp * gauss;
						*weight += gauss;
					}


				}
			}
		}
	});

	// normalization of the confidence values by the sum of weights
	tbb::parallel_for(0u, height, [&](int y) {
		for(unsigned x=0; x<width; ++x) {
			float& val = corresp.at<float>(y, x);
			float& w = weights.at<float>(y, x);

			if(w > 0.0f)
				val /= w;
		}
	});

	return corresp;
}

} }
