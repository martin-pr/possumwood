#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in, a_mask;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat mat;

	if(data.get(a_mask).empty())
		cv::bitwise_not(*data.get(a_in), mat);
	else
		cv::bitwise_not(*data.get(a_in), mat, *data.get(a_mask));

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mask, "mask", possumwood::opencv::Frame());
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_mask, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/bitwise/not", init);

}  // namespace
