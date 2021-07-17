#include <tbb/parallel_for.h>
#include <opencv2/opencv.hpp>

#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include <lightfields/slic_superpixels_2d.h>

#include "frame.h"

namespace {

// Implementation of the SLIC superpixels algorithm
// Achanta, Radhakrishna, et al. "SLIC superpixels compared to state-of-the-art superpixel methods." IEEE transactions
// on pattern analysis and machine intelligence 34.11 (2012): 2274-2282.

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<unsigned> a_targetPixelCount;
dependency_graph::InAttr<float> a_spatialBias;
dependency_graph::InAttr<unsigned> a_iterations;
dependency_graph::InAttr<possumwood::Enum> a_filter;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

enum FilterMode { kNone, kComponentsFinalize, kComponentsEachIteration };

static std::vector<std::pair<std::string, int>> s_filterMode{
    {"none", kNone},
    {"connected components, final step", kComponentsFinalize},
    {"connected components, each iteration", kComponentsEachIteration},
};

template <typename T>
dependency_graph::State computeT(dependency_graph::Values& data) {
	const cv::Mat& in = *data.get(a_inFrame);

	// compute the superpixel spacing S
	const int S = lightfields::SlicSuperpixels2D<T>::initS(in.rows, in.cols, data.get(a_targetPixelCount));

	// start with a regular grid of superpixels
	auto pixels = lightfields::SlicSuperpixels2D<T>::initPixels(in, S);

	// create a Metric instance based on input parameters
	const typename lightfields::SlicSuperpixels2D<T>::Metric metric(S, data.get(a_spatialBias));

	// build the labelling and metric value matrices
#ifndef NDEBUG
	{
		std::atomic<typename lightfields::SlicSuperpixels2D<T>::Label> tmp(
		    typename lightfields::SlicSuperpixels2D<T>::Label(0, 0));
		assert(tmp.is_lock_free() && "Atomics in this instance make sense only if they're lock free.");
	}
#endif

	lightfields::Grid<typename lightfields::SlicSuperpixels2D<T>::Label> labels(in.rows, in.cols);

	// make sure labelling happens even with 0 iteration count
	bool firstStep = true;
	lightfields::SlicSuperpixels2D<T>::label(in, labels, pixels, metric);

	for(unsigned i = 0; i < data.get(a_iterations); ++i) {
		// using the metric instance, label all pixels
		if(!firstStep)
			lightfields::SlicSuperpixels2D<T>::label(in, labels, pixels, metric);
		else
			firstStep = false;

		// perform the filtering, if selected
		if(data.get(a_filter).intValue() == kComponentsEachIteration)
			lightfields::SlicSuperpixels2D<T>::connectedComponents(labels, pixels);

		// recompute centres as means of all labelled pixels
		lightfields::SlicSuperpixels2D<T>::findCenters(in, labels, pixels);
	}

	// address all disconnected components
	if(data.get(a_filter).intValue() == kComponentsFinalize)
		lightfields::SlicSuperpixels2D<T>::connectedComponents(labels, pixels);

	// copy all to a cv::Mat
	cv::Mat result = cv::Mat::zeros(in.rows, in.cols, CV_32SC1);
	tbb::parallel_for(0, in.rows, [&](int y) {
		for(int x = 0; x < in.cols; ++x)
			result.at<int>(y, x) = labels(y, x).id;
	});
	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

dependency_graph::State compute(dependency_graph::Values& data) {
	if(data.get(a_targetPixelCount) == 0)
		throw std::runtime_error("Only positive pixel counts are allowed.");

	const cv::Mat& in = *data.get(a_inFrame);
	switch(in.type()) {
		case CV_8UC3:
			return computeT<unsigned char>(data);
		case CV_32FC3:
			return computeT<float>(data);
		default:
			throw std::runtime_error("Only CV_8UC3 type supported on input for the moment!");
	}
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_targetPixelCount, "target_pixel_count", 2000u);
	meta.addAttribute(a_spatialBias, "spatial_bias", 1.0f);
	meta.addAttribute(a_iterations, "iterations", 10u);
	meta.addAttribute(a_filter, "filter",
	                  possumwood::Enum(s_filterMode.begin(), s_filterMode.end(), kComponentsFinalize));
	meta.addAttribute(a_outFrame, "superpixels", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_targetPixelCount, a_outFrame);
	meta.addInfluence(a_spatialBias, a_outFrame);
	meta.addInfluence(a_iterations, a_outFrame);
	meta.addInfluence(a_filter, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/superpixels/slic", init);

}  // namespace
