#include "nearest_integration.h"

#include <tbb/parallel_for.h>

#include "samples.h"

namespace lightfields {
namespace nearest {

IntegrationResult integrate(const lightfields::Samples& samples, const Imath::Vec2<unsigned>& size, float offset) {
	const unsigned width = size[0];
	const unsigned height = size[1];

	// TODO: for parallelization to work reliably, we need to use integer atomics here, unfortunately

	cv::Mat average = cv::Mat::zeros(height, width, CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(height, width, CV_16UC3);

	const float x_scale = (float)width / (float)samples.sensorSize()[0];
	const float y_scale = (float)height / (float)samples.sensorSize()[1];

	const float widthf = width;
	const float heightf = height;

	const tbb::blocked_range<lightfields::Samples::const_iterator> range(samples.begin(), samples.end());

	tbb::parallel_for(range, [&](const tbb::blocked_range<lightfields::Samples::const_iterator> range) {
		for(const lightfields::Samples::Sample& sample : range) {
			const float target_x = (sample.xy[0] + offset * sample.uv[0]) * x_scale;
			const float target_y = (sample.xy[1] + offset * sample.uv[1]) * y_scale;

			if((target_x >= 0.0f) && (target_y >= 0.0f) && (target_x < widthf) && (target_y < heightf)) {
				float* color = average.ptr<float>(floor(target_y), floor(target_x));
				uint16_t* n = norm.ptr<uint16_t>(floor(target_y), floor(target_x));

				if(sample.color == lightfields::Samples::kRGB)
					for(int a = 0; a < 3; ++a) {
						color[a] += sample.value[a];
						++n[a];
					}
				else {
					color[sample.color] += sample.value[sample.color];
					++n[sample.color];
				}
			}
		}
	});

	tbb::parallel_for(0, average.rows, [&](int y) {
		for(int x = 0; x < average.cols; ++x)
			for(int a = 0; a < 3; ++a)
				if(norm.ptr<uint16_t>(y, x)[a] > 0)
					average.ptr<float>(y, x)[a] /= (float)norm.ptr<uint16_t>(y, x)[a];
	});

	return IntegrationResult{average, norm};
}  // namespace nearest

cv::Mat correspondence(const lightfields::Samples& samples, const IntegrationResult& integration, float offset) {
	const unsigned width = integration.average.cols;
	const unsigned height = integration.average.rows;

	cv::Mat corresp = cv::Mat::zeros(height, width, CV_32FC1);

	const float x_scale = (float)width / (float)samples.sensorSize()[0];
	const float y_scale = (float)height / (float)samples.sensorSize()[1];

	const float widthf = width;
	const float heightf = height;

	// accumulate the sums for std dev
	tbb::parallel_for(
	    tbb::blocked_range<lightfields::Samples::const_iterator>(samples.begin(), samples.end()),
	    [&](const tbb::blocked_range<lightfields::Samples::const_iterator>& range) {
		    for(const lightfields::Samples::Sample& sample : range) {
			    const float target_x = (sample.xy[0] + offset * sample.uv[0]) * x_scale;
			    const float target_y = (sample.xy[1] + offset * sample.uv[1]) * y_scale;

			    if((target_x >= 0.0f) && (target_y >= 0.0f) && (target_x < widthf) && (target_y < heightf)) {
				    float* target = corresp.ptr<float>(floor(target_y), floor(target_x));
				    const float* ave = integration.average.ptr<float>(target_y, target_x);

				    if(sample.color == lightfields::Samples::kRGB)
					    for(int c = 0; c < 3; ++c) {
						    const float tmp = sample.value[c] - ave[c];  // because pow() is expensive?!
						    *target += tmp * tmp;
					    }
				    else {
					    const float tmp = (sample.value[sample.color] - ave[sample.color]);
					    *target += tmp * tmp;
				    }
			    }
		    }
	    });

	// normalization
	tbb::parallel_for(0u, height, [&](unsigned y) {
		for(unsigned x = 0; x < width; ++x) {
			float* target = corresp.ptr<float>(y, x);
			const uint16_t* n = integration.samples.ptr<uint16_t>(y, x);

			uint16_t norm = 0;
			for(int c = 0; c < 3; ++c)
				norm += n[c];

			if(norm > 0)
				*target = (*target / (float)(norm));
			else
				*target = 0.0f;
		}
	});

	return corresp;
}

}  // namespace nearest
}  // namespace lightfields
