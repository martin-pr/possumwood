#pragma once

#include <opencv2/opencv.hpp>
#include <ImathVec.h>

#include "integration.h"

namespace lightfields {

class Samples;

namespace gaussian {

/// Integrates an image - produces an image using gaussian integration
IntegrationResult integrate(const lightfields::Samples& samples, const Imath::Vec2<unsigned>& size, const cv::Mat& data, float sigma2 = 2.0f, float offset = 0.0f);

/// Compute the correspondence (integration variance) metric on a previously-integrated result
// cv::Mat correspondence(const lightfields::Samples& samples, const cv::Mat& data, const Result& integration, float offset = 0.0f);

} }
