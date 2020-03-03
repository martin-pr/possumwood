#include "depth.h"

#include <tbb/parallel_for.h>

namespace lightfields {

namespace {

class Curve {
	public:
		struct Value {
			float depth, confidence;
		};

		Curve(std::size_t s) : m_values(s) {
			assert(s >= 2);
		}

		typedef std::vector<Value>::iterator iterator;

		iterator begin() {
			return m_values.begin();
		}

		iterator end() {
			return m_values.end();
		}

		std::pair<Value, Value> minima() const {
			// this is actually not correct - assuming that first and last elements are minima

			// handling of extrema in the first and last element
			std::pair<Value, Value> result;
			if(m_values.front().confidence > m_values.back().confidence) {
				result.first = m_values.back();
				result.second = m_values.front();
			}
			else {
				result.first = m_values.front();
				result.second = m_values.back();
			}

			// extrema in middle elements
			if(m_values.size() > 2)
				for(auto it = m_values.begin()+1; it != m_values.end()-1; ++it) {
					const bool isMinimum = (it->confidence <= (it-1)->confidence) && (it->confidence <= (it+1)->confidence);
					if(isMinimum) {
						if(result.first.confidence > it->confidence) {
							result.second = result.first;
							result.first = *it;
						}
						else if(result.second.confidence > it->confidence) {
							result.second = *it;
						}
					}
				}

			return result;
		}

	private:
		std::vector<Value> m_values;
};

}

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
		cv::min(m_layers[uv_offset], confidence, m_layers[uv_offset]);
}

Depth::Result Depth::eval() const {
	assert(!m_layers.empty());

	const int width = m_layers.begin()->second.cols;
	const int height = m_layers.begin()->second.rows;

	cv::Mat result(height, width, CV_32FC1);
	cv::Mat conf(height, width, CV_32FC1);

	tbb::parallel_for(tbb::blocked_range<int>(0, height), [&](const tbb::blocked_range<int>& range) {
		Curve curve(m_layers.size());

		for(int y=range.begin(); y != range.end(); ++y) {
			for(int x=0; x<width; ++x) {
				auto ci = curve.begin();
				auto it = m_layers.begin();

				while(it != m_layers.end()) {
					*ci = Curve::Value { it->first, it->second.at<float>(y, x) };

					++ci;
					++it;
				}

				auto vals = curve.minima();

				result.at<float>(y, x) = vals.first.depth;
				conf.at<float>(y, x) = vals.second.confidence / vals.first.confidence;
			}
		}
	});

	return Result{result, conf};
}

bool Depth::empty() const {
	return m_layers.empty();
}

}

