#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"

namespace {

int mode(const std::string& s) {
	if(s == "INTER_NEAREST")
		return cv::INTER_NEAREST;
	if(s == "INTER_LINEAR")
		return cv::INTER_LINEAR;
	if(s == "INTER_AREA")
		return cv::INTER_AREA;
	if(s == "INTER_CUBIC")
		return cv::INTER_CUBIC;
	if(s == "INTER_LANCZOS4")
		return cv::INTER_LANCZOS4;

	assert(false);
	return -1;
}

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<float> a_scale;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result;

	cv::resize(*data.get(a_inFrame), result, cv::Size(), data.get(a_scale), data.get(a_scale),
	           mode(data.get(a_mode).value()));

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_scale, "scale", 1.0f);
	meta.addAttribute(a_mode, "mode",
	                  possumwood::Enum(
	                      {
	                          "INTER_NEAREST",
	                          "INTER_LINEAR",
	                          "INTER_AREA",
	                          "INTER_CUBIC",
	                          "INTER_LANCZOS4",
	                      },
	                      cv::INTER_LINEAR));

	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_scale, a_outFrame);
	meta.addInfluence(a_mode, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/resize", init);

}  // namespace
