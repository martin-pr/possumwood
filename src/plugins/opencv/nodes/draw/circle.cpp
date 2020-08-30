#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<float> a_centerX, a_centerY, a_color;
dependency_graph::InAttr<unsigned> a_radius, a_thickness;
dependency_graph::InAttr<bool> a_fill;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result = (*data.get(a_inFrame)).clone();
	cv::circle(result, cv::Point(data.get(a_centerX), data.get(a_centerY)), data.get(a_radius),
	           cv::Scalar(data.get(a_color)), data.get(a_fill) ? -1 : data.get(a_thickness));

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_centerX, "center/x", 0.0f);
	meta.addAttribute(a_centerY, "center/y", 0.0f);
	meta.addAttribute(a_color, "color", 127.0f);
	meta.addAttribute(a_radius, "radius", 10u);
	meta.addAttribute(a_thickness, "thickness", 2u);
	meta.addAttribute(a_fill, "fill", false);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_centerX, a_outFrame);
	meta.addInfluence(a_centerY, a_outFrame);
	meta.addInfluence(a_color, a_outFrame);
	meta.addInfluence(a_radius, a_outFrame);
	meta.addInfluence(a_thickness, a_outFrame);
	meta.addInfluence(a_fill, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/draw/circle", init);

}  // namespace
