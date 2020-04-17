#include <possumwood_sdk/node_implementation.h>

#include <atomic>

#include <opencv2/opencv.hpp>
#include <tbb/parallel_for.h>

#include <actions/traits.h>
#include <lightfields/grid.h>
#include <lightfields/bitfield.h>

#include "frame.h"

namespace {

// Implementation of the SLIC superpixels algorithm
// Achanta, Radhakrishna, et al. "SLIC superpixels compared to state-of-the-art superpixel methods." IEEE transactions on pattern analysis and machine intelligence 34.11 (2012): 2274-2282.

struct Center {
	Center() : row(0), col(0), color{{0, 0, 0}} {
	}

	Center(const cv::Mat& m, int r, int c) : row(r), col(c) {
		assert(r >= 0 && r < m.rows);
		assert(c >= 0 && c < m.cols);

		auto ptr = m.ptr<unsigned char>(r, c);
		for(int a=0;a<3;++a)
			color[a] = ptr[a];
	}

	Center& operator +=(const Center& c) {
		row += c.row;
		col += c.col;

		for(int a=0;a<3;++a)
			color[a] += c.color[a];

		return *this;
	}

	Center& operator /=(int div) {
		row /= div;
		col /= div;

		for(int a=0;a<3;++a)
			color[a] /= div;

		return *this;
	}

	int row, col;
	std::array<int, 3> color;
};

class Metric {
	public:
		/// S is the superpixel distance; m is the spatial-to-color weight
		Metric(float S, float m) : m_SS(S*S), m_mm(m*m) {
		}

		// implementation of eq. 3 of the paper
		float operator()(const Center& c, const cv::Mat& m, const int row, const int col) const {
			float d_c = 0.0f;
			for(int a=0;a<3;++a) {
				float elem = float(c.color[a]) - float(m.ptr<unsigned char>(row, col)[a]);
				elem *= elem;

				d_c += elem;
			}

			const float d_s = float(c.row - row)*float(c.row - row) + float(c.col - col)*float(c.col - col);

			return std::sqrt(d_c + d_s / m_SS * m_mm);
		}

	private:
		float m_SS, m_mm;
};

struct Pixel {
	Pixel(int lbl = 0, float metr = std::numeric_limits<float>::max()) noexcept : label(lbl), metric(metr) {
	}

	int label;
	float metric;
};

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<unsigned> a_targetPixelCount;
dependency_graph::InAttr<float> a_spatialBias;
dependency_graph::InAttr<unsigned> a_iterations;
dependency_graph::InAttr<bool> a_filter;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& in = *data.get(a_inFrame);
	if(in.type() != CV_8UC3)
		throw std::runtime_error("Only CV_8UC3 type supported on input for the moment!");

	// compute the superpixel spacing S
	const int S = sqrt((in.cols * in.rows) / data.get(a_targetPixelCount));
	if(S <= 0)
		throw std::runtime_error("Too many superpixels for the size of input image!");

	// start with a regular grid of superpixels
	lightfields::Grid<Center> pixels(0, 0);
	{
		const int rows = in.rows / S;
		const int cols = in.cols / S;

		pixels = lightfields::Grid<Center>(rows, cols);

		for(int y=0; y<rows; ++y)
			for(int x=0; x<cols; ++x)
				pixels(y, x) = Center(in, (in.rows * y) / rows + S/2, (in.cols * x) / cols + S/2);
	}

	// create a Metric instance based on input parameters
	const Metric metric(S, data.get(a_spatialBias));

	// build the labelling and metric value matrices
#ifndef NDEBUG
	{
		std::atomic<Pixel> tmp(Pixel(0, 0));
		assert(tmp.is_lock_free() && "Atomics in this instance make sense only if they're lock free.");
	}
#endif

	lightfields::Grid<std::atomic<Pixel>> labels(in.rows, in.cols);

	for(unsigned i=0; i<data.get(a_iterations); ++i) {
		// using the metric instance, label all pixels
		tbb::parallel_for(0, int(pixels.container().size()), [&](int a) {
			const Center& center = pixels.container()[a];

			for(int row = std::max(0, center.row-S); row < std::min(in.rows, center.row+S+1); ++row)
				for(int col = std::max(0, center.col-S); col < std::min(in.cols, center.col+S+1); ++col) {
					const Pixel next(a, metric(center, in, row, col));
					std::atomic<Pixel>& current = labels(row, col);

					Pixel tmp(0, 0);
					do {
						tmp = current;
					} while(tmp.metric > next.metric && !current.compare_exchange_weak(tmp, next));
				}
		});

		// recompute centres as means of all labelled pixels
		{
			std::vector<Center> sum(pixels.container().size());
			std::vector<int> count(pixels.container().size(), 0);

			for(int row=0; row<in.rows; ++row)
				for(int col=0; col<in.cols; ++col) {
					const int label = labels(row, col).load().label;
					assert(label >= 0 && label < (int)sum.size());

					sum[label] += Center(in, row, col);
					count[label]++;
				}

			for(std::size_t a=0; a<sum.size(); ++a)
				if(count[a] > 0) {
					sum[a] /= count[a];
					pixels.container()[a] = sum[a];
				}
		}
	}

	// address all disconnected components
	{
		// first, find all disconnected components
		lightfields::Grid<bool, lightfields::Bitfield> connected(in.rows, in.cols);
		std::vector<unsigned> counters(pixels.container().size());

		tbb::parallel_for(std::size_t(0), pixels.container().size(), [&](std::size_t a) {
			static thread_local std::vector<std::pair<int, int>> stack;
			assert(stack.empty());

			stack.push_back(std::make_pair(pixels.container()[a].row, pixels.container()[a].col));
			while(!stack.empty()) {
				const std::pair<int, int> current = stack.back();
				stack.pop_back();

				assert(current.first < in.rows);
				assert(current.second < in.cols);

				if(!connected(current.first, current.second) && labels(current.first, current.second).load().label == (int)a) {
					connected(current.first, current.second) = true;

					++counters[a];

					if(current.first > 0)
						stack.push_back(std::make_pair(current.first-1, current.second));
					if(current.first < in.rows-1)
						stack.push_back(std::make_pair(current.first+1, current.second));
					if(current.second > 0)
						stack.push_back(std::make_pair(current.first, current.second-1));
					if(current.second < in.cols-1)
						stack.push_back(std::make_pair(current.first, current.second+1));
				}
			}
		});

		for(std::size_t a=0;a<counters.size();++a)
			std::cout << a << "  ->  " << counters[a] << std::endl; // THERE SHOULD BE NO ZEROES HERE - any zero means that the center is OUTSIDE the labelled pixels, bad!

		// if(data.get(a_filter)) {
		// 	for(int row=0; row<in.rows; ++row)
		// 		for(int col=0; col<in.cols; ++col)
		// 			labels(row, col) = Pixel(connected(row, col));
		// }

		// for each disconnected component pixel
		if(data.get(a_filter)) {
			// find a disconnected pixel
			for(int row=0; row<in.rows; ++row)
				for(int col=0; col<in.cols; ++col) {
					if(!connected(row, col)) {
						// map the component's pixels and its surrounding
						std::vector<std::pair<int, int>> component;
						std::map<int, int> neighboursCounter;

						{
							std::vector<std::pair<int, int>> stack;
							stack.push_back(std::make_pair(row, col));

							while(!stack.empty()) {
								const std::pair<int, int> current = stack.back();
								stack.pop_back();

								if(!connected(current.first, current.second)) {
									connected(current.first, current.second) = true;
									component.push_back(current);

									if(current.first > 0)
										stack.push_back(std::make_pair(current.first-1, current.second));
									if(current.first < in.rows-1)
										stack.push_back(std::make_pair(current.first+1, current.second));
									if(current.second > 0)
										stack.push_back(std::make_pair(current.first, current.second-1));
									if(current.second < in.cols-1)
										stack.push_back(std::make_pair(current.first, current.second+1));
								}

								else
									neighboursCounter[labels(current.first, current.second).load().label]++;
							}
						}

						// find the label with most items
						int label = 0;
						{
							int ctr = 0;
							for(const auto& n : neighboursCounter)
								if(n.second > ctr) {
									ctr = n.second;
									label = n.first;
								}
						}

						// and assign it to all the pixels of this component
						for(const auto& p : component)
							labels(p.first, p.second) = Pixel(label);
					}
				}

		}
	}

	// copy all to a cv::Mat
	cv::Mat result = cv::Mat::zeros(in.rows, in.cols, CV_32SC1);
	tbb::parallel_for(0, in.rows, [&](int y) {
		for(int x=0; x<in.cols; ++x)
			result.at<int>(y, x) = labels(y, x).load().label;
	});
	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_targetPixelCount, "target_pixel_count", 2000u);
	meta.addAttribute(a_spatialBias, "spatial_bias", 1.0f);
	meta.addAttribute(a_iterations, "iterations", 10u);
	meta.addAttribute(a_filter, "filter", true);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_targetPixelCount, a_outFrame);
	meta.addInfluence(a_spatialBias, a_outFrame);
	meta.addInfluence(a_iterations, a_outFrame);
	meta.addInfluence(a_filter, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/slic_superpixels", init);

}
