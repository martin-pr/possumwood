#include <possumwood_sdk/node_implementation.h>

#include <atomic>

#include <opencv2/opencv.hpp>
#include <tbb/parallel_for.h>

#include <actions/traits.h>
#include <lightfields/slic_superpixels.h>
#include <possumwood_sdk/datatypes/enum.h>

#include "frame.h"

namespace {

// Implementation of the SLIC superpixels algorithm
// Achanta, Radhakrishna, et al. "SLIC superpixels compared to state-of-the-art superpixel methods." IEEE transactions on pattern analysis and machine intelligence 34.11 (2012): 2274-2282.

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<unsigned> a_targetPixelCount;
dependency_graph::InAttr<float> a_spatialBias;
dependency_graph::InAttr<unsigned> a_iterations;
dependency_graph::InAttr<possumwood::Enum> a_filter;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

enum FilterMode {
	kNone,
	kComponentsFinalize,
	kComponentsEachIteration
};

static std::vector<std::pair<std::string, int>> s_filterMode {
	{"none", kNone},
	{"connected components, final step", kComponentsFinalize},
	{"connected components, each iteration", kComponentsEachIteration},
};

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& in = *data.get(a_inFrame);
	if(in.type() != CV_8UC3)
		throw std::runtime_error("Only CV_8UC3 type supported on input for the moment!");

	if(data.get(a_targetPixelCount) == 0)
		throw std::runtime_error("Only positive pixel counts are allowed.");

	// compute the superpixel spacing S
	const int S = lightfields::SlicSuperpixels::initS(in.rows, in.cols, data.get(a_targetPixelCount));

	// start with a regular grid of superpixels
	auto pixels = lightfields::SlicSuperpixels::initPixels(in, S);

	// create a Metric instance based on input parameters
	const lightfields::SlicSuperpixels::Metric metric(S, data.get(a_spatialBias));

	// build the labelling and metric value matrices
#ifndef NDEBUG
	{
		std::atomic<lightfields::SlicSuperpixels::Label> tmp(lightfields::SlicSuperpixels::Label(0, 0));
		assert(tmp.is_lock_free() && "Atomics in this instance make sense only if they're lock free.");
	}
#endif

	lightfields::Grid<std::atomic<lightfields::SlicSuperpixels::Label>> labels(in.rows, in.cols);

	// make sure labelling happens even with 0 iteration count
	bool firstStep = true;
	lightfields::SlicSuperpixels::label(in, labels, pixels, metric);

	for(unsigned i=0; i<data.get(a_iterations); ++i) {
		// using the metric instance, label all pixels
		if(!firstStep)
			lightfields::SlicSuperpixels::label(in, labels, pixels, metric);
		else
			firstStep = false;

		// perform the filtering, if selected
		if(data.get(a_filter).intValue() == kComponentsEachIteration)
			lightfields::SlicSuperpixels::connectedComponents(labels, pixels);

		// recompute centres as means of all labelled pixels
		lightfields::SlicSuperpixels::findCenters(in, labels, pixels);
	}

	// address all disconnected components
	if(data.get(a_filter).intValue() == kComponentsFinalize)
		lightfields::SlicSuperpixels::connectedComponents(labels, pixels);

	// copy all to a cv::Mat
	cv::Mat result = cv::Mat::zeros(in.rows, in.cols, CV_32SC1);
	tbb::parallel_for(0, in.rows, [&](int y) {
		for(int x=0; x<in.cols; ++x)
			result.at<int>(y, x) = labels(y, x).load().id;
	});
	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_targetPixelCount, "target_pixel_count", 2000u);
	meta.addAttribute(a_spatialBias, "spatial_bias", 1.0f);
	meta.addAttribute(a_iterations, "iterations", 10u);
	meta.addAttribute(a_filter, "filter", possumwood::Enum(s_filterMode.begin(), s_filterMode.end(), kComponentsFinalize));
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
