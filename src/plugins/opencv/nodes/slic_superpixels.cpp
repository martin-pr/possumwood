#include <possumwood_sdk/node_implementation.h>

#include <atomic>

#include <opencv2/opencv.hpp>
#include <tbb/parallel_for.h>

#include <actions/traits.h>
#include <lightfields/grid.h>
#include <lightfields/bitfield.h>
#include <lightfields/slic_superpixels.h>

#include "frame.h"

namespace {

// Implementation of the SLIC superpixels algorithm
// Achanta, Radhakrishna, et al. "SLIC superpixels compared to state-of-the-art superpixel methods." IEEE transactions on pattern analysis and machine intelligence 34.11 (2012): 2274-2282.
class Metric {
	public:
		/// S is the superpixel distance; m is the spatial-to-color weight
		Metric(float S, float m) : m_SS(S*S), m_mm(m*m) {
		}

		// implementation of eq. 3 of the paper
		float operator()(const lightfields::SlicSuperpixels::Center& c, const cv::Mat& m, const int row, const int col) const {
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

	if(data.get(a_targetPixelCount) == 0)
		throw std::runtime_error("Only positive pixel counts are allowed.");

	// compute the superpixel spacing S
	const int S = lightfields::SlicSuperpixels::initS(in.rows, in.cols, data.get(a_targetPixelCount));

	// start with a regular grid of superpixels
	lightfields::Grid<lightfields::SlicSuperpixels::Center> pixels(0, 0);
	{
		const int rows = in.rows / S;
		const int cols = in.cols / S;

		pixels = lightfields::Grid<lightfields::SlicSuperpixels::Center>(rows, cols);

		for(int y=0; y<rows; ++y)
			for(int x=0; x<cols; ++x)
				pixels(y, x) = lightfields::SlicSuperpixels::Center(in, (in.rows * y) / rows + S/2, (in.cols * x) / cols + S/2);
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
			const lightfields::SlicSuperpixels::Center& center = pixels.container()[a];

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
			std::vector<lightfields::SlicSuperpixels::Center> sum(pixels.container().size());
			std::vector<int> count(pixels.container().size(), 0);

			for(int row=0; row<in.rows; ++row)
				for(int col=0; col<in.cols; ++col) {
					const int label = labels(row, col).load().label;
					assert(label >= 0 && label < (int)sum.size());

					sum[label] += lightfields::SlicSuperpixels::Center(in, row, col);
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
	if(data.get(a_filter)) {
		// first, find all components for each label
		//  label - component - pixels (pair of ints)
		std::vector<std::vector<std::vector<std::pair<int, int>>>> components(pixels.container().size());

		{
			lightfields::Grid<bool, lightfields::Bitfield> connected(in.rows, in.cols);

			for(int y=0; y<in.rows; ++y)
				for(int x=0; x<in.cols; ++x) {
					// found a new component
					if(!connected(y, x)) {
						// recursively map it
						const int currentLabel = labels(y, x).load().label;

						components[currentLabel].push_back(std::vector<std::pair<int, int>>());
						std::vector<std::pair<int, int>>& currentComponent = components[currentLabel].back();

						std::vector<std::pair<int, int>> stack;
						stack.push_back(std::make_pair(y, x));

						while(!stack.empty()) {
							const std::pair<int, int> current = stack.back();
							stack.pop_back();

							assert(current.first < in.rows);
							assert(current.second < in.cols);

							if(!connected(current.first, current.second) && labels(current.first, current.second).load().label == currentLabel) {
								connected(current.first, current.second) = true;

								currentComponent.push_back(std::make_pair(current.first, current.second));

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
					}
				}
		}

		// the largest component is THE component for the label - let's remove it from the list of components to process
		lightfields::Grid<bool, lightfields::Bitfield> processed(in.rows, in.cols);
		for(std::size_t a=0; a<components.size(); ++a) {
			auto& label = components[a];

			if(!label.empty()) {
				std::size_t maxSize = 0;
				std::size_t maxIndex = 0;

				for(int i=0; i<(int)label.size(); ++i)
					if(label[i].size() > maxSize) {
						maxIndex = i;
						maxSize = label[i].size();
					}

				// mark the component as "processed"
				for(auto& p : label[maxIndex])
					processed(p.first, p.second) = true;

				// and then remove the component from the label list
				if(maxSize > 0)
					label.erase(label.begin() + maxIndex);
			}
		}

		// finally, go through all the remaining components and relabel them based on surrounding labels
		// -> in some cases, the unlabelled components can be "nested" (especially in the presence of noise).
		//    We need to run this algorithm iteratively until we managed to label everything.
		std::size_t unprocessed = 1;
		while(unprocessed > 0) {
			unprocessed = 0;

			for(std::size_t label_id=0; label_id<components.size(); ++label_id) {
				auto& label = components[label_id];

				for(auto& comp : label) {
					std::vector<unsigned> counter(components.size());

					for(auto& p : comp) {
						if(p.first > 0 && processed(p.first-1, p.second))
							counter[labels(p.first-1, p.second).load().label]++;
						if(p.first < in.rows-1 && processed(p.first+1, p.second))
							counter[labels(p.first+1, p.second).load().label]++;
						if(p.second > 0 && processed(p.first, p.second-1))
							counter[labels(p.first, p.second-1).load().label]++;
						if(p.second < in.cols-1 && processed(p.first, p.second+1))
							counter[labels(p.first, p.second+1).load().label]++;
					}

					// find the most prelevant resolved label (there might not be any!)
					std::size_t maxLabel = 0;
					unsigned maxCount = 0;

					for(std::size_t a=0;a<counter.size();++a)
						if(maxCount < counter[a]) {
							maxCount = counter[a];
							maxLabel = a;
						}

					if(maxCount > 0) {
						// and label all the pixels with this label
						for(auto& p : comp) {
							labels(p.first, p.second) = Pixel(maxLabel);
							processed(p.first, p.second) = true;
						}
					}
					else
						unprocessed++;
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
