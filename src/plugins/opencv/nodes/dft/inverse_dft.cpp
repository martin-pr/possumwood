#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat input = *data.get(a_inFrame);

	if(input.type() != CV_32FC2)
		throw std::runtime_error("Only CV_32FC2 type allowed on input.");

	cv::Mat output;

	// DFT_COMPLEX_INPUT doesn't exist in OpenCV 1.11.14 in Bionic, but with 2-channel input, it is implied
	dft(input, output, /*cv::DFT_COMPLEX_INPUT |*/ cv::DFT_REAL_OUTPUT | cv::DFT_INVERSE | cv::DFT_SCALE);
	assert(output.type() == CV_32FC1);

	data.set(a_outFrame, possumwood::opencv::Frame(output));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/dft/inverse_dft", init);

}  // namespace
