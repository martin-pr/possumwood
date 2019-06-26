#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in1, a_in2;
dependency_graph::InAttr<float> a_alpha;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat mat;

	if(data.get(a_alpha) < 0.0f || data.get(a_alpha) > 1.0f)
		throw std::runtime_error("Alpha value should be in range 0..1");

	cv::addWeighted(*data.get(a_in1), data.get(a_alpha), *data.get(a_in2), 1.0f - data.get(a_alpha), 0.0f, mat);

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in1, "in_frame_1", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_in2, "in_frame_2", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_alpha, "alpha", 0.5f);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in1, a_out);
	meta.addInfluence(a_in2, a_out);
	meta.addInfluence(a_alpha, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/blend", init);

}
