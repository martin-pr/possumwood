#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame1, a_inFrame2;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

int modeToEnum(const std::string& mode) {
	if(mode == "CMP_EQ")
		return cv::CMP_EQ;
	else if(mode == "CMP_GT")
		return cv::CMP_GT;
	else if(mode == "CMP_GE")
		return cv::CMP_GE;
	else if(mode == "CMP_LT")
		return cv::CMP_LT;
	else if(mode == "CMP_LE")
		return cv::CMP_LE;
	else if(mode == "CMP_NE")
		return cv::CMP_NE;

	throw std::runtime_error("Unknown comparison mode " + mode);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result;

	cv::compare(*data.get(a_inFrame1), *data.get(a_inFrame2), result, modeToEnum(data.get(a_mode).value()));

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame1, "in_frame_1", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_inFrame2, "in_frame_2", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode",
		possumwood::Enum({"CMP_EQ", "CMP_GT", "CMP_GE", "CMP_LT", "CMP_LE", "CMP_NE"}));
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame1, a_outFrame);
	meta.addInfluence(a_inFrame2, a_outFrame);
	meta.addInfluence(a_mode, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/compare", init);

}
