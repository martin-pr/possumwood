#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include "vec2.h"

namespace lightfields {

/// A simple implementation of Markov Random Field on a regular grid, with integer values
class MRF {
	public:
		struct Value {
			Value(int val = 0, float conf = 0.0f) : value(val), confidence(conf) {
			}

			int value;
			float confidence;
		};

		MRF(const V2i& size);

		Value& operator[](const V2i& index);
		const Value& operator[](const V2i& index) const;

		const V2i& size() const;

		cv::Mat solveICM(float inputsWeight, float flatnessWeight, float smoothnessWeight, std::size_t iterationLimit) const;

	private:
		V2i m_size;
		std::vector<Value> m_nodes;
};

}
