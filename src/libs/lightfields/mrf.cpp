#include "mrf.h"

#include <cassert>
#include <cmath>
#include <limits>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>

#include "pmf.h"
#include "grid.h"
#include "pdf_gaussian.h"

namespace lightfields {

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

}

///

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

std::pair<int, int> MRF::range() const {
	MinMax minmax((*this)[V2i(0, 0)].value);
	for(int y=0;y<(*this).size().y;++y)
		for(int x=0;x<(*this).size().x;++x)
			minmax.add((*this)[V2i(x, y)].value);

	return std::make_pair(minmax.min, minmax.max);
}

//////////////////////////////////////////////////////////////////

namespace {

float evalICM(const MRF& source, const cv::Mat& state, const V2i& pos, float inputsWeight, float flatnessWeight, float smoothnessWeight, const Neighbours& neighbours) {
	// first find the min and max candidates
	MinMax minmax(source[pos].value);
	minmax.add(state.at<unsigned char>(pos.y, pos.x));
	neighbours.eval(pos, [&](const V2i& pos, float weight) {
		minmax.add(state.at<unsigned char>(pos.y, pos.x));
	});

	// get the min energy value
	float energy = std::numeric_limits<float>::max();
	int label = state.at<unsigned char>(pos.y, pos.x);
	for(int val = minmax.min; val <= minmax.max; ++val) {
		// inputs term
		const int e_inputs = std::abs(source[pos].value - val);

		// flatness term
		float e_flat = 0, e_flat_norm = 0;
		neighbours.eval(pos, [&](const V2i& pos, float weight) {
			e_flat += std::abs(val - state.at<unsigned char>(pos.y, pos.x)) * weight;
			e_flat_norm += weight;
		});

		// smoothness term (laplacian)
		float e_smooth = 0, e_smooth_norm = 0;
		neighbours.eval(pos, [&](const V2i& pos, float weight) {
			e_smooth += (val - state.at<unsigned char>(pos.y, pos.x)) * weight;
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

cv::Mat MRF::solveICM(const MRF& source, float inputsWeight, float flatnessWeight, float smoothnessWeight, std::size_t iterationLimit, const Neighbours& neighbourhood) {
	cv::Mat state = cv::Mat::zeros(source.size().y, source.size().x, CV_8UC1);

	cv::Mat result = cv::Mat::zeros(source.size().y, source.size().x, CV_8UC1);
	for(int y=0;y<result.rows;++y)
		for(int x=0;x<result.cols;++x)
			result.at<unsigned char>(y, x) = source[V2i(x, y)].value;

	for(std::size_t it=0; it<iterationLimit; ++it) {
		cv::swap(result, state);

		tbb::parallel_for(tbb::blocked_range2d<int>(0, source.size().y, 0, source.size().x), [&](const tbb::blocked_range2d<int>& range) {
			for(int y=range.rows().begin(); y != range.rows().end(); ++y)
				for(int x=range.cols().begin(); x != range.cols().end(); ++x)
					result.at<unsigned char>(y, x) = evalICM(source, state, V2i(x, y), inputsWeight, flatnessWeight, smoothnessWeight, neighbourhood);
		});
	}

	return result;
}

///////////////////////

cv::Mat MRF::solvePropagation(const MRF& source, float inputsWeight, float flatnessWeight, float smoothnessWeight, std::size_t iterationLimit, const Neighbours& neighbourhood) {
	// find the range of values
	const std::pair<int, int> minmax = source.range();

	// build a grid of probability mass functions
	Grid<PMF> grid(source.size().y, source.size().x, PMF(minmax.second+1));
	for(int y=0;y<source.size().y;++y)
		for(int x=0;x<source.size().x;++x)
			grid(y, x) = PMF::fromConfidence(source[V2i(x, y)].confidence, source[V2i(x, y)].value, minmax.second+1);

	// todo: the main algorithm
	const JointPMF diff = JointPMF::difference(minmax.second+1);

	Grid<PMF> state = grid;
	for(std::size_t it=0; it<iterationLimit; ++it) {
		grid.swap(state);

		tbb::parallel_for(tbb::blocked_range2d<unsigned>(0u, grid.rows(), 0u, grid.cols()), [&](const tbb::blocked_range2d<unsigned>& range) {
			for(unsigned y=range.rows().begin(); y != range.rows().end(); ++y)
				for(unsigned x=range.cols().begin(); x != range.cols().end(); ++x) {
					PMF current = PMF::fromConfidence(source[V2i(x, y)].confidence, source[V2i(x, y)].value, minmax.second+1);

					// PMF flatness = PMF(minmax.second+1);
					// float norm = 0.0f;

					// neighbourhood.eval(V2i(x, y), [&](const V2i& pos, float weight) {
					// 	flatness = PMF::combine(flatness, norm, state(pos.y, pos.x), weight);
					// 	norm += weight;
					// });

					PMF flatness = PMF(minmax.second+1);
					neighbourhood.eval(V2i(x, y), [&](const V2i& pos, float weight) {
						flatness = flatness * state(pos.y, pos.x);
					});

					current = PMF::combine(current, inputsWeight, flatness, flatnessWeight);

					grid(y, x) = current;
				}
		});
	}

	// convert the result to a cv::Mat by picking the highest probability for each pixel
	cv::Mat result = cv::Mat::zeros(grid.rows(), grid.cols(), CV_8UC1);
	for(unsigned y=0;y<grid.rows();++y)
		for(unsigned x=0;x<grid.cols();++x)
			result.at<unsigned char>(y, x) = grid(y, x).max();
	return result;
}

cv::Mat MRF::solvePDF(const MRF& source, float inputsWeight, float flatnessWeight, float smoothnessWeight, std::size_t iterationLimit, const Neighbours& neighbourhood) {
	// find the range of values
	const std::pair<int, int> minmax = source.range();

	// build a grid of probability mass functions
	Grid<PDFGaussian> grid(source.size().y, source.size().x, PDFGaussian(0, 0));
	for(int y=0;y<source.size().y;++y)
		for(int x=0;x<source.size().x;++x)
			grid(y, x) = PDFGaussian::fromPeak(source[V2i(x, y)].value, source[V2i(x, y)].confidence);

	// todo: the main algorithm
	const JointPMF diff = JointPMF::difference(minmax.second+1);

	Grid<PDFGaussian> state = grid;
	for(std::size_t it=0; it<iterationLimit; ++it) {
		grid.swap(state);

		tbb::parallel_for(tbb::blocked_range2d<unsigned>(0u, grid.rows(), 0u, grid.cols()), [&](const tbb::blocked_range2d<unsigned>& range) {
			for(unsigned y=range.rows().begin(); y != range.rows().end(); ++y)
				for(unsigned x=range.cols().begin(); x != range.cols().end(); ++x) {
					const PDFGaussian constness = PDFGaussian::fromPeak(source[V2i(x, y)].value, source[V2i(x, y)].confidence) - state(y, x);

					PDFGaussian flatness = PDFGaussian(0,0);
					float norm = 0.0f;
					neighbourhood.eval(V2i(x, y), [&](const V2i& pos, float weight) {
						const PDFGaussian tmp = (state(pos.y, pos.x) - state(y, x));   // if ONE of these has 0 confidence, the result is 0 confidence! Zero confidence cells get NEVER UPDATED! FIX!!!

						flatness = flatness + tmp * weight * tmp.confidence();

						norm += weight * tmp.confidence();
					});

					if(norm > 0.0f)
						flatness = flatness / norm;

					PDFGaussian current(0, 0);
					norm = 0.0f;
					if(!std::isinf(constness.sigma()) && inputsWeight > 0.0f) {
						current = current + constness * inputsWeight;
						norm += inputsWeight;
					}

					if(!std::isinf(flatness.sigma()) && flatnessWeight > 0.0f) {
						current = current + flatness * flatnessWeight;
						norm += flatnessWeight;
					}

					if(!std::isinf(state(y, x).sigma()))
						grid(y, x) = state(y, x) + current;
					else if(norm > 0.0f)
						grid(y, x) = current / norm;
					else
						grid(y, x) = state(y, x);
				}
		});
	}

	// convert the result to a cv::Mat by picking the highest probability for each pixel
	cv::Mat result = cv::Mat::zeros(grid.rows(), grid.cols(), CV_8UC1);
	for(unsigned y=0;y<grid.rows();++y)
		for(unsigned x=0;x<grid.cols();++x)
			result.at<unsigned char>(y, x) = grid(y, x).mu();
	return result;
}

}
