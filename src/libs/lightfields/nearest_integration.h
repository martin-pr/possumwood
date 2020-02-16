#pragma once

#include <opencv2/opencv.hpp>
#include <ImathVec.h>

namespace lightfields {

class Samples;

namespace nearest {

struct Result {
	cv::Mat average; ///< resulting RGB image (float, 3 channels)
	cv::Mat samples; ///< per-pixel sample count (unsigned int 16, 3 channels)
};

/// Integrates an image - produces an image using nearest-neighbour integration
Result integrate(const lightfields::Samples& samples, const Imath::Vec2<unsigned>& size, const cv::Mat& data);

/// Compute the correspondence (integration variance) metric on a previously-integrated result
cv::Mat correspondence(const lightfields::Samples& samples, const cv::Mat& data, const Result& integration);

} }
