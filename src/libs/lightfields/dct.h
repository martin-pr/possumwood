#pragma once

#include <opencv2/opencv.hpp>

#include "samples.h"

namespace lightfields {

class DCT {
  public:
	DCT() = default;

	std::array<float, 3> get(float x, float y, float u, float v) const;

  private:
	std::vector<std::vector<cv::Mat>> m_data;
	unsigned m_xySamples, m_uvSamples;

	friend DCT dct(const Samples& samples, unsigned xy_samples, unsigned uv_samples);
};

DCT dct(const Samples& samples, unsigned xy_samples, unsigned uv_samples);

std::ostream& operator<<(std::ostream& out, const lightfields::DCT& dct);

}  // namespace lightfields
