#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_prevFrame, a_nextFrame;
// dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::InAttr<float> a_pyrScale, a_polySigma;
dependency_graph::InAttr<unsigned> a_levels, a_winSize, a_iterations, a_polyN;
dependency_graph::InAttr<possumwood::Enum> a_flags;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

int flagsToEnum(const std::string& mode) {
	if(mode == "USE_INITIAL_FLOW")
		return cv::OPTFLOW_USE_INITIAL_FLOW;
	else if(mode == "FARNEBACK_GAUSSIAN")
		return cv::OPTFLOW_FARNEBACK_GAUSSIAN;

	throw std::runtime_error("Unknown optical flow flags " + mode);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result;

	cv::calcOpticalFlowFarneback(*data.get(a_prevFrame), *data.get(a_nextFrame), result, data.get(a_pyrScale),
	                             data.get(a_levels), data.get(a_winSize), data.get(a_iterations), data.get(a_polyN),
	                             data.get(a_polySigma), flagsToEnum(data.get(a_flags).value()));

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_prevFrame, "prev_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_nextFrame, "next_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_pyrScale, "pyramid/scale", 0.5f);
	meta.addAttribute(a_levels, "pyramid/levels", 1u);
	meta.addAttribute(a_winSize, "window_size", 21u);
	meta.addAttribute(a_iterations, "iterations", 5u);
	meta.addAttribute(a_polyN, "polygon/neighbourhood", 7u);
	meta.addAttribute(a_polySigma, "polygon/sigma", 1.5f);

	meta.addAttribute(a_flags, "flags", possumwood::Enum({"USE_INITIAL_FLOW", "FARNEBACK_GAUSSIAN"}));

	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_prevFrame, a_outFrame);
	meta.addInfluence(a_nextFrame, a_outFrame);
	meta.addInfluence(a_pyrScale, a_outFrame);
	meta.addInfluence(a_levels, a_outFrame);
	meta.addInfluence(a_winSize, a_outFrame);
	meta.addInfluence(a_iterations, a_outFrame);
	meta.addInfluence(a_polyN, a_outFrame);
	meta.addInfluence(a_polySigma, a_outFrame);
	meta.addInfluence(a_flags, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/optical_flow/dense", init);

}  // namespace
