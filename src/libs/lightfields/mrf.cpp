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

struct Neighbours_4 {
	template<typename FN>
	static void eval(const V2i& size, const V2i& pos, const cv::Mat& state, const FN& fn) {
		if(pos.x > 0)
			fn(state.at<unsigned char>(pos.y, pos.x-1), 1);
		if(pos.x < size.x-1)
			fn(state.at<unsigned char>(pos.y, pos.x+1), 1);
		if(pos.y > 0)
			fn(state.at<unsigned char>(pos.y-1, pos.x), 1);
		if(pos.y < size.y-1)
			fn(state.at<unsigned char>(pos.y+1, pos.x), 1);
	}
};

struct Neighbours_8 {
	template<typename FN>
	static void eval(const V2i& size, const V2i& pos, const cv::Mat& state, const FN& fn) {
		int min_x = std::max(0, pos.x-1);
		int min_y = std::max(0, pos.y-1);
		int max_x = std::min(pos.x+1, size.x-1);
		int max_y = std::min(pos.y+1, size.y-1);

		for(int y = min_y; y <= max_y; ++y)
			for(int x = min_x; x <= max_x; ++x)
				if(x != pos.x || y != pos.y)
					fn(state.at<unsigned char>(y, x), 1);
	}
};

struct Neighbours_8_Weighted {
	template<typename FN>
	static void eval(const V2i& size, const V2i& pos, const cv::Mat& state, const FN& fn) {
		int min_x = std::max(0, pos.x-1);
		int min_y = std::max(0, pos.y-1);
		int max_x = std::min(pos.x+1, size.x-1);
		int max_y = std::min(pos.y+1, size.y-1);

		for(int y = min_y; y <= max_y; ++y)
			for(int x = min_x; x <= max_x; ++x)
				if(x != pos.x || y != pos.y)
					fn(state.at<unsigned char>(y, x), 1 + (x == pos.x || y == pos.y));
	}
};
template<class NEIGHBOURS>
float evalICM(const MRF& source, const cv::Mat& state, const V2i& pos, float inputsWeight, float flatnessWeight, float smoothnessWeight) {
	// first find the min and max candidates
	MinMax minmax(source[pos].value);
	minmax.add(state.at<unsigned char>(pos.y, pos.x));
	NEIGHBOURS::eval(source.size(), pos, state, [&](int n, int weight) {
		minmax.add(n);
	});

	// get the min energy value
	float energy = std::numeric_limits<float>::max();
	int label = state.at<unsigned char>(pos.y, pos.x);
	for(int val = minmax.min; val <= minmax.max; ++val) {
		// inputs term
		const int e_inputs = std::abs(source[pos].value - val);

		// flatness term
		int e_flat = 0, e_flat_norm = 0;
		NEIGHBOURS::eval(source.size(), pos, state, [&](int n, int weight) {
			e_flat += std::abs(val - n) * weight;
			e_flat_norm += weight;
		});

		// smoothness term (laplacian)
		int e_smooth = 0, e_smooth_norm = 0;
		NEIGHBOURS::eval(source.size(), pos, state, [&](int n, int weight) {
			e_smooth += (val - n) * weight;
			++e_smooth_norm += weight;
		});

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

cv::Mat MRF::solveICM(float inputsWeight, float flatnessWeight, float smoothnessWeight, std::size_t iterationLimit, ICMNeighbourhood neighbourhood) const {
	auto evaluate = &evalICM<Neighbours_4>;
	if(neighbourhood == k8)
		evaluate = &evalICM<Neighbours_8>;
	else if(neighbourhood == k8Weighted)
		evaluate = &evalICM<Neighbours_8_Weighted>;

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
					result.at<unsigned char>(y, x) = evaluate(*this, state, V2i(x, y), inputsWeight, flatnessWeight, smoothnessWeight);
				}
		});
	}

	return result;
}

}
