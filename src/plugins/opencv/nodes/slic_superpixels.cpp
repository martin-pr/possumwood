#include <possumwood_sdk/node_implementation.h>

#include <atomic>

#include <opencv2/opencv.hpp>
#include <tbb/parallel_for.h>

#include <actions/traits.h>
#include <lightfields/grid.h>

#include "frame.h"

namespace {

// Implementation of the SLIC superpixels algorithm
// Achanta, Radhakrishna, et al. "SLIC superpixels compared to state-of-the-art superpixel methods." IEEE transactions on pattern analysis and machine intelligence 34.11 (2012): 2274-2282.

struct Center {
	Center(const cv::Mat& m, int r, int c) : row(r), col(c) {
		auto ptr = m.ptr<unsigned char>(r, c);
		for(int a=0;a<3;++a)
			color[a] = ptr[a];
	}

	int row, col;
	unsigned char color[3];
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
	std::vector<Center> pixels;
	{
		const int ofx_y = (in.rows - (((in.rows-1) / S) * S)) / 2;
		const int ofx_x = (in.cols - (((in.cols-1) / S) * S)) / 2;

		for(int y=0; y<=(in.rows-1) / S; ++y)
			for(int x=0; x<=(in.cols-1) / S; ++x)
				pixels.push_back(Center(in, y*S + ofx_y, x*S + ofx_x));
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

	// using the metric instance, label all pixels
	tbb::parallel_for(0, int(pixels.size()), [&](int a) {
		const Center& center = pixels[a];

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
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_targetPixelCount, a_outFrame);
	meta.addInfluence(a_spatialBias, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/slic_superpixels", init);

}
