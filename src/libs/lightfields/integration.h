#pragma once

namespace lightfields {

struct IntegrationResult {
	cv::Mat average; ///< resulting RGB image (float, 3 channels)
	cv::Mat samples; ///< per-pixel sample count (unsigned int 16, 3 channels)
};

}
