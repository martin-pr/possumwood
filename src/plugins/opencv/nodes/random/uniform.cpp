#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<float> a_low, a_high;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;


dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result = (*data.get(a_inFrame)).clone();

	cv::randu(result, data.get(a_low), data.get(a_high));

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_low, "low", 0.0f);
	meta.addAttribute(a_high, "high", 255.0f);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_low, a_outFrame);
	meta.addInfluence(a_high, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/random/uniform", init);

}
