#include "nearest_integration.h"

#include <tbb/parallel_for.h>

#include "samples.h"

namespace lightfields { namespace nearest {

namespace {

void add1channel(float* color, uint16_t* n, int colorIndex, const float* value) {
	color[colorIndex] += value[0];
	++n[colorIndex];
}

void add3channel(float* color, uint16_t* n, int colorIndex, const float* value) {
	color[0] += value[0];
	++n[0];

	color[1] += value[1];
	++n[1];

	color[2] += value[2];
	++n[2];
}

}

IntegrationResult integrate(const lightfields::Samples& samples, const Imath::Vec2<unsigned>& size, const cv::Mat& input, float offset) {
	if(input.type() != CV_32FC3 && input.type() != CV_32FC1)
		throw std::runtime_error("Nearest neighbour integration - only 1 or 3 channel float data allowed on input.");

	auto applyFn = &add1channel;
	if(input.type() == CV_32FC3)
		applyFn = &add3channel;

	const unsigned width = size[0];
	const unsigned height = size[1];

	// TODO: for parallelization to work reliably, we need to use integer atomics here, unfortunately

	cv::Mat average = cv::Mat::zeros(height, width, CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(height, width, CV_16UC3);

	const float x_scale = (float)width / (float)samples.sensorSize()[0];
	const float y_scale = (float)height / (float)samples.sensorSize()[1];

	const float widthf = width;
	const float heightf = height;

	tbb::parallel_for(0, input.rows, [&](int y) {
		const auto begin = samples.begin(y);
		const auto end = samples.end(y);
		assert(begin <= end);

		for(auto it = begin; it != end; ++it) {
			assert(it->source[1] == y);

			const float target_x = (it->xy[0] + offset * it->uv[0]) * x_scale;
			const float target_y = (it->xy[1] + offset * it->uv[1]) * y_scale;

			// sanity checks
			assert(it->source[0] < input.rows);
			assert(it->source[1] < input.cols);

			if((target_x >= 0.0f) && (target_y >= 0.0f) && (target_x < widthf) && (target_y < heightf)) {
				float* color = average.ptr<float>(floor(target_y), floor(target_x));
				uint16_t* n = norm.ptr<uint16_t>(floor(target_y), floor(target_x));

				const float* value = input.ptr<float>(it->source[1], it->source[0]);
				(*applyFn)(color, n, it->color, value);
			}
		}
	});

	tbb::parallel_for(0, average.rows, [&](int y) {
		for(int x=0;x<average.cols;++x)
			for(int a=0;a<3;++a)
				if(norm.ptr<uint16_t>(y,x)[a] > 0)
					average.ptr<float>(y,x)[a] /= (float)norm.ptr<uint16_t>(y,x)[a];
	});

	return IntegrationResult { average, norm };
}

cv::Mat correspondence(const lightfields::Samples& samples, const cv::Mat& input, const IntegrationResult& integration, float offset) {
	if(input.type() != CV_32FC1 && input.type() != CV_32FC3)
		throw std::runtime_error("Only 32-bit single-float or 32-bit 3 channel float format supported on input.");

	const unsigned width = integration.average.cols;
	const unsigned height = integration.average.rows;

	cv::Mat corresp = cv::Mat::zeros(height, width, CV_32FC1);

	const float x_scale = (float)width / (float)samples.sensorSize()[0];
	const float y_scale = (float)height / (float)samples.sensorSize()[1];

	const float widthf = width;
	const float heightf = height;

	tbb::parallel_for(0, input.rows, [&](int y) {
		const auto end = samples.end(y);
		const auto begin = samples.begin(y);
		assert(begin <= end);

		for(auto it = begin; it != end; ++it) {
			const float target_x = (it->xy[0] + offset * it->uv[0]) * x_scale;
			const float target_y = (it->xy[1] + offset * it->uv[1]) * y_scale;

			if((target_x >= 0.0f) && (target_y >= 0.0f) && (target_x < widthf) && (target_y < heightf)) {
				float* target = corresp.ptr<float>(floor(target_y), floor(target_x));
				const float* value = input.ptr<float>(it->source[1], it->source[0]);
				const float* ave = integration.average.ptr<float>(target_y, target_x);
				const uint16_t* n = integration.samples.ptr<uint16_t>(target_y, target_x);

				if(input.channels() == 3)
					for(int c=0; c<3; ++c) {
						const float tmp = value[c] - ave[c]; // because pow() is expensive?!
						*target += tmp*tmp / (float)n[c];
					}
				else {
					const float tmp = *value - ave[it->color];
					*target += tmp*tmp / (float)n[it->color];
				}
			}
		}
	});

	return corresp;
}

} }
