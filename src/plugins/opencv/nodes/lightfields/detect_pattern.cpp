#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& input = *data.get(a_in);
	if(input.type() != CV_8UC1)
		throw std::runtime_error("Only CV_8UC1 type allowed on input");

	cv::Mat mat(input.size(), CV_8UC1, 0);

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/detect_pattern", init);

}
