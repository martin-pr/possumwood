#include "mrf.h"

#include <cassert>
#include <cmath>
#include <limits>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>

namespace lightfields {

MRF::MRF(const V2i& size) : m_size(size), m_nodes(m_size.x * m_size.y) {
}

MRF::Value& MRF::operator[](const V2i& index) {
	assert(index.x >= 0 && index.x < m_size.x);
	assert(index.y >= 0 && index.y < m_size.y);

	return m_nodes[index.x + index.y * m_size.y];
}

const MRF::Value& MRF::operator[](const V2i& index) const {
	assert(index.x >= 0 && index.x < m_size.x);
	assert(index.y >= 0 && index.y < m_size.y);

	return m_nodes[index.x + index.y * m_size.y];
}

const V2i& MRF::size() const {
	return m_size;
}

//////////////////////////////////////////////////////////////////

namespace {

struct MinMax {
	MinMax(int val) : min(val), max(val) {
	}

	void add(int val) {
		min = std::min(min, val);
		max = std::max(max, val);
	}

	int min, max;
};

float evalICM(const MRF& source, const cv::Mat& state, const V2i& pos, float inputsWeight, float flatnessWeight, float smoothnessWeight) {
	// first find the min and max candidates
	MinMax minmax(source[pos].value);
	minmax.add(state.at<unsigned char>(pos.y, pos.x));
	if(pos.x > 0)
		minmax.add(state.at<unsigned char>(pos.y, pos.x-1));
	if(pos.x < source.size().x-1)
		minmax.add(state.at<unsigned char>(pos.y, pos.x+1));
	if(pos.y > 0)
		minmax.add(state.at<unsigned char>(pos.y-1, pos.x));
	if(pos.y < source.size().y-1)
		minmax.add(state.at<unsigned char>(pos.y+1, pos.x));

	// get the min energy value
	float energy = std::numeric_limits<float>::max();
	int label = state.at<unsigned char>(pos.y, pos.x);
	for(int val = minmax.min; val <= minmax.max; ++val) {
		// inputs term
		const int e_inputs = std::abs(source[pos].value - val);

		// flatness term
		int e_flat = 0, e_flat_norm = 0;
		if(pos.x > 0) {
			e_flat += std::abs(val - state.at<unsigned char>(pos.y, pos.x-1));
			++e_flat_norm;
		}
		if(pos.x < source.size().x-1) {
			e_flat += std::abs(val - state.at<unsigned char>(pos.y, pos.x+1));
			++e_flat_norm;
		}
		if(pos.y > 0) {
			e_flat += std::abs(val - state.at<unsigned char>(pos.y-1, pos.x));
			++e_flat_norm;
		}
		if(pos.y < source.size().y-1) {
			e_flat += std::abs(val - state.at<unsigned char>(pos.y+1, pos.x));
			++e_flat_norm;
		}

		// smoothness term (laplacian)
		int e_smooth = 0, e_smooth_norm = 0;
		if(pos.x > 0) {
			e_smooth += val - state.at<unsigned char>(pos.y, pos.x-1);
			++e_smooth_norm;
		}
		if(pos.x < source.size().x-1) {
			e_smooth += val - state.at<unsigned char>(pos.y, pos.x+1);
			++e_smooth_norm;
		}
		if(pos.y > 0) {
			e_smooth += val - state.at<unsigned char>(pos.y-1, pos.x);
			++e_smooth_norm;
		}
		if(pos.y < source.size().y-1) {
			e_smooth += val - state.at<unsigned char>(pos.y+1, pos.x);
			++e_smooth_norm;
		}

		// putting them all together
		const float e =
			source[pos].confidence * inputsWeight * (float)e_inputs                 // pulls towards the observed value with given confidence
			+ flatnessWeight * ((float)e_flat / (float)e_flat_norm)                 // minimizes the first derivative
			+ smoothnessWeight * ((float)std::abs(e_smooth) / (float)e_smooth_norm) // minimizes the second derivative
		;

		if(e < energy) {
			energy = e;
			label = val;
		}
	}

	return label;
}

}

cv::Mat MRF::solveICM(float inputsWeight, float flatnessWeight, float smoothnessWeight, std::size_t iterationLimit) const {
	cv::Mat state = cv::Mat::zeros(m_size.y, m_size.x, CV_8UC1);

	cv::Mat result = cv::Mat::zeros(m_size.y, m_size.x, CV_8UC1);
	for(int y=0;y<result.rows;++y)
		for(int x=0;x<result.cols;++x)
			result.at<unsigned char>(y, x) = (*this)[V2i(x, y)].value;

	for(std::size_t it=0; it<iterationLimit; ++it) {
		cv::swap(result, state);

		tbb::parallel_for(tbb::blocked_range2d<int>(0, m_size.y, 0, m_size.x), [&](const tbb::blocked_range2d<int>& range) {
			for(int y=range.rows().begin(); y != range.rows().end(); ++y)
				for(int x=range.cols().begin(); x != range.cols().end(); ++x) {
					result.at<unsigned char>(y, x) = evalICM(*this, state, V2i(x, y), inputsWeight, flatnessWeight, smoothnessWeight);
				}
		});
	}

	return result;
}

}
