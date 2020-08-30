#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in1, a_in2, a_mask;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat mat;

	if(data.get(a_mask).empty())
		cv::bitwise_xor(*data.get(a_in1), *data.get(a_in2), mat);
	else
		cv::bitwise_xor(*data.get(a_in1), *data.get(a_in2), mat, *data.get(a_mask));

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in1, "in_frame_1", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_in2, "in_frame_2", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mask, "mask", possumwood::opencv::Frame());
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in1, a_out);
	meta.addInfluence(a_in2, a_out);
	meta.addInfluence(a_mask, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/bitwise/xor", init);

}  // namespace
