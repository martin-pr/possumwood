#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame, a_inMask;
dependency_graph::InAttr<float> a_radius;
dependency_graph::InAttr<possumwood::Enum> a_method;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

int methodToEnum(const std::string& method) {
	if(method == "INPAINT_NS")
		return cv::INPAINT_NS;
	else if(method == "INPAINT_TELEA")
		return cv::INPAINT_TELEA;

	throw std::runtime_error("Unknown inpaint method " + method);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result;

	cv::inpaint(*data.get(a_inFrame), *data.get(a_inMask), result, data.get(a_radius), methodToEnum(data.get(a_method).value()));

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_inMask, "mask", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_radius, "radius", 5.0f);
	meta.addAttribute(a_method, "method", possumwood::Enum({"INPAINT_NS", "INPAINT_TELEA"}));
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_inMask, a_outFrame);
	meta.addInfluence(a_radius, a_outFrame);
	meta.addInfluence(a_method, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/inpaint", init);

}
