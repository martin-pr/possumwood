#pragma once

#include "nearest_integration.h"

namespace lightfields {

class Depth {
	public:
		Depth();

		/// Adds a layer of correspondences. Can be called multiple times with the same uv_offset - will simply pick the highest level of confidence.
		void addLayer(float uv_offset, const cv::Mat& confidence);

		/// evaluate the confidence levels - returns the highest level per-pixel
		cv::Mat eval() const;

		bool empty() const;

	private:
		std::map<float, cv::Mat> m_layers;
};


}
