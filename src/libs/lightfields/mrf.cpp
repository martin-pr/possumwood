#include "mrf.h"

#include <cassert>
#include <cmath>
#include <limits>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>

#include "pmf.h"

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
	void eval(const V2i& pos, const cv::Mat& state, const FN& fn) const {
		if(pos.x > 0)
			fn(state.at<unsigned char>(pos.y, pos.x-1), 1);
		if(pos.x < state.cols-1)
			fn(state.at<unsigned char>(pos.y, pos.x+1), 1);
		if(pos.y > 0)
			fn(state.at<unsigned char>(pos.y-1, pos.x), 1);
		if(pos.y < state.rows-1)
			fn(state.at<unsigned char>(pos.y+1, pos.x), 1);
	}
};

struct Neighbours_8 {
	template<typename FN>
	void eval(const V2i& pos, const cv::Mat& state, const FN& fn) const {
		int min_x = std::max(0, pos.x-1);
		int min_y = std::max(0, pos.y-1);
		int max_x = std::min(pos.x+1, state.cols-1);
		int max_y = std::min(pos.y+1, state.rows-1);

		for(int y = min_y; y <= max_y; ++y)
			for(int x = min_x; x <= max_x; ++x)
				if(x != pos.x || y != pos.y)
					fn(state.at<unsigned char>(y, x), 1);
	}
};

struct Neighbours_8_Weighted {
	template<typename FN>
	void eval(const V2i& pos, const cv::Mat& state, const FN& fn) const {
		int min_x = std::max(0, pos.x-1);
		int min_y = std::max(0, pos.y-1);
		int max_x = std::min(pos.x+1, state.cols-1);
		int max_y = std::min(pos.y+1, state.rows-1);

		for(int y = min_y; y <= max_y; ++y)
			for(int x = min_x; x <= max_x; ++x)
				if(x != pos.x || y != pos.y)
					fn(state.at<unsigned char>(y, x), 1 + (x == pos.x || y == pos.y));
	}
};
template<class NEIGHBOURS>
float evalICM(const MRF& source, const cv::Mat& state, const V2i& pos, float inputsWeight, float flatnessWeight, float smoothnessWeight, const NEIGHBOURS& neighbours) {
	// first find the min and max candidates
	MinMax minmax(source[pos].value);
	minmax.add(state.at<unsigned char>(pos.y, pos.x));
	neighbours.eval(pos, state, [&](int n, float weight) {
		minmax.add(n);
	});

	// get the min energy value
	float energy = std::numeric_limits<float>::max();
	int label = state.at<unsigned char>(pos.y, pos.x);
	for(int val = minmax.min; val <= minmax.max; ++val) {
		// inputs term
		const int e_inputs = std::abs(source[pos].value - val);

		// flatness term
		float e_flat = 0, e_flat_norm = 0;
		neighbours.eval(pos, state, [&](int n, float weight) {
			e_flat += std::abs(val - n) * weight;
			e_flat_norm += weight;
		});

		// smoothness term (laplacian)
		float e_smooth = 0, e_smooth_norm = 0;
		neighbours.eval(pos, state, [&](int n, float weight) {
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

cv::Mat MRF::solveICM(const MRF& source, float inputsWeight, float flatnessWeight, float smoothnessWeight, std::size_t iterationLimit, ICMNeighbourhood neighbourhood) {
	using namespace std::placeholders;

	std::function<float(const MRF&, const cv::Mat&, const V2i&, float, float, float)> evaluate;
	if(neighbourhood == k4)
		evaluate = std::bind(evalICM<Neighbours_4>, _1, _2, _3, _4, _5, _6, Neighbours_4());
	if(neighbourhood == k8)
		evaluate = std::bind(evalICM<Neighbours_8>, _1, _2, _3, _4, _5, _6, Neighbours_8());
	else if(neighbourhood == k8Weighted)
		evaluate = std::bind(evalICM<Neighbours_8_Weighted>, _1, _2, _3, _4, _5, _6, Neighbours_8_Weighted());
	assert(evaluate);

	cv::Mat state = cv::Mat::zeros(source.size().y, source.size().x, CV_8UC1);

	cv::Mat result = cv::Mat::zeros(source.size().y, source.size().x, CV_8UC1);
	for(int y=0;y<result.rows;++y)
		for(int x=0;x<result.cols;++x)
			result.at<unsigned char>(y, x) = source[V2i(x, y)].value;

	for(std::size_t it=0; it<iterationLimit; ++it) {
		cv::swap(result, state);

		tbb::parallel_for(tbb::blocked_range2d<int>(0, source.size().y, 0, source.size().x), [&](const tbb::blocked_range2d<int>& range) {
			for(int y=range.rows().begin(); y != range.rows().end(); ++y)
				for(int x=range.cols().begin(); x != range.cols().end(); ++x) {
					result.at<unsigned char>(y, x) = evaluate(source, state, V2i(x, y), inputsWeight, flatnessWeight, smoothnessWeight);
				}
		});
	}

	return result;
}

///////////////////////

namespace {

class Grid {
	public:
		Grid(unsigned rows, unsigned cols, unsigned layers) : m_rows(rows), m_cols(cols), m_layers(layers), m_p(rows*cols, PMF(layers)) {
		}

		PMF& operator() (unsigned row, unsigned col) {
			assert(row < m_rows && col < m_cols);
			return m_p[row * m_cols + col];
		}

		const PMF& operator() (unsigned row, unsigned col) const {
			assert(row < m_rows && col < m_cols);
			return m_p[row * m_cols + col];
		}

		unsigned rows() const {
			return m_rows;
		}

		unsigned cols() const {
			return m_cols;
		}

		void swap(Grid& g) {
			std::swap(g.m_rows, m_rows);
			std::swap(g.m_cols, m_cols);
			std::swap(g.m_layers, m_layers);

			m_p.swap(g.m_p);
		}

	private:
		unsigned m_rows, m_cols, m_layers;
		std::vector<PMF> m_p;
};

}

cv::Mat MRF::solvePropagation(const MRF& source, float inputsWeight, float flatnessWeight, float smoothnessWeight, std::size_t iterationLimit) {
	// find the range of values
	MinMax minmax(0);
	for(int y=0;y<source.size().y;++y)
		for(int x=0;x<source.size().x;++x)
			minmax.add(source[V2i(x, y)].value);

	// build a grid of probability mass functions
	Grid grid(source.size().y, source.size().x, minmax.max+1);
	for(int y=0;y<source.size().y;++y)
		for(int x=0;x<source.size().x;++x)
			grid(y, x) = PMF::fromConfidence(source[V2i(x, y)].confidence, source[V2i(x, y)].value, minmax.max+1);

	// todo: the main algorithm
	const JointPMF diff = JointPMF::difference(minmax.max+1);

	Grid state = grid;
	for(std::size_t it=0; it<iterationLimit; ++it) {
		grid.swap(state);

		tbb::parallel_for(tbb::blocked_range2d<unsigned>(0u, grid.rows(), 0u, grid.cols()), [&](const tbb::blocked_range2d<unsigned>& range) {
			for(unsigned y=range.rows().begin(); y != range.rows().end(); ++y)
				for(unsigned x=range.cols().begin(); x != range.cols().end(); ++x) {
					PMF current = PMF::fromConfidence(source[V2i(x, y)].confidence, source[V2i(x, y)].value, minmax.max+1);

					PMF flatness = PMF(minmax.max+1);
					float norm = 0.0f;
					if(x > 0) {
						flatness = PMF::combine(flatness, norm, state(y, x-1), 1.0f);
						norm += 1.0f;
					}
					if(x < grid.cols()-1) {
						flatness = PMF::combine(flatness, norm, state(y, x+1), 1.0f);
						norm += 1.0f;
					}
					if(y > 0) {
						flatness = PMF::combine(flatness, norm, state(y-1, x), 1.0f);
						norm += 1.0f;
					}
					if(y < grid.rows()-1)
						flatness = PMF::combine(flatness, norm, state(y+1, x), 1.0f);

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

}
