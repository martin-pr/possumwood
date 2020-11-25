#include <possumwood_sdk/node_implementation.h>

#include <mutex>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "laplacian_inpainting.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame, a_inMask;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State state;

	std::vector<cv::Mat> inputs;
	inputs.push_back(*data.get(a_inFrame));

	std::vector<cv::Mat> masks;
	masks.push_back(*data.get(a_inMask));

	std::vector<cv::Mat> result;

	state.append(possumwood::opencv::inpaint(inputs, masks, result));
	assert(result.size() == 1);

	data.set(a_outFrame, possumwood::opencv::Frame(result[0]));

	return state;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_inMask, "mask", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_inMask, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/inpaint_laplacian", init);

}  // namespace
