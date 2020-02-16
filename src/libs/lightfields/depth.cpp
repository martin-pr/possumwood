#include "depth.h"

#include <tbb/parallel_for.h>

namespace lightfields {

Depth::Depth() {
}

void Depth::addLayer(float uv_offset, const cv::Mat& confidence) {
	assert(confidence.type() == CV_32FC1 && "Confidence should be a single-channel float value");

	assert(m_layers.empty() || m_layers.begin()->second.cols == confidence.cols);
	assert(m_layers.empty() || m_layers.begin()->second.rows == confidence.rows);

	auto it = m_layers.find(uv_offset);
	if(it == m_layers.end())
		m_layers[uv_offset] = confidence.clone();
	else
		cv::max(m_layers[uv_offset], confidence, m_layers[uv_offset]);
}

cv::Mat Depth::eval() const {
	assert(!m_layers.empty());

	const int width = m_layers.begin()->second.cols;
	const int height = m_layers.begin()->second.rows;

	cv::Mat result(height, width, CV_32FC1);

	tbb::parallel_for(0, height, [&](int y) {
		for(int x=0; x<width; ++x) {
			auto it = m_layers.begin();

			float val = it->first;
			float confidence = it->second.at<float>(y, x);

			++it;
			for(; it != m_layers.end(); ++it) {
				float tmp = it->second.at<float>(y, x);
				if(tmp < confidence) {
					val = it->first;
					confidence = tmp;
				}
			}

			result.at<float>(y, x) = val;
		}
	});

	return result;
}

bool Depth::empty() const {
	return m_layers.empty();
}

}

