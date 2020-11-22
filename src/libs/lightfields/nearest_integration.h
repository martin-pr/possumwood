#pragma once

#include <OpenEXR/ImathVec.h>

#include <opencv2/opencv.hpp>

#include "integration.h"

namespace lightfields {

class Samples;

namespace nearest {

/// Integrates an image - produces an image using nearest-neighbour integration
IntegrationResult integrate(const lightfields::Samples& samples,
                            const Imath::Vec2<unsigned>& size,
                            float offset = 0.0f);

/// Compute the correspondence (integration variance) metric on a previously-integrated result
cv::Mat correspondence(const lightfields::Samples& samples, const IntegrationResult& integration, float offset = 0.0f);

}  // namespace nearest
}  // namespace lightfields
