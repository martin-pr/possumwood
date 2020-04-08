#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include "neighbours.h"

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

		std::pair<int, int> range() const;

		static cv::Mat solveICM(const MRF& source, float inputsWeight, float flatnessWeight, float smoothnessWeight, std::size_t iterationLimit, const Neighbours& neighbourhood);
		static cv::Mat solvePropagation(const MRF& source, float inputsWeight, float flatnessWeight, float smoothnessWeight, std::size_t iterationLimit, const Neighbours& neighbourhood);
		static cv::Mat solvePDF(const MRF& source, float inputsWeight, float flatnessWeight, float smoothnessWeight, std::size_t iterationLimit, const Neighbours& neighbourhood);

	private:
		V2i m_size;
		std::vector<Value> m_nodes;
};

}
