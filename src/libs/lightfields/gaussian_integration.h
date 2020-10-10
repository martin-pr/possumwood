#pragma once

#include <OpenEXR/ImathVec.h>

#include <opencv2/opencv.hpp>

#include "integration.h"

namespace lightfields {

class Samples;

namespace gaussian {

/// Integrates an image - produces an image using gaussian integration
IntegrationResult integrate(const lightfields::Samples& samples, const Imath::Vec2<unsigned>& size, const cv::Mat& data,
                            float sigma = 4.0f, float offset = 0.0f);

/// Compute the correspondence (integration variance) metric on a previously-integrated result
cv::Mat correspondence(const lightfields::Samples& samples, const cv::Mat& data, const IntegrationResult& integration,
                       float sigma = 4.0f, float offset = 0.0f);

}  // namespace gaussian
}  // namespace lightfields
