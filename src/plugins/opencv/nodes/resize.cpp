#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<float> a_scale;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result;

	cv::resize(*data.get(a_inFrame), result, cv::Size(), data.get(a_scale), data.get(a_scale));

	data.set(a_outFrame, possumwood::opencv::Frame(result));
	
	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_scale, "scale", 1.0f);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_scale, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/resize", init);

}
