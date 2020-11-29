#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include <tbb/task_group.h>

#include <opencv2/opencv.hpp>

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
	const cv::Mat& input = *data.get(a_inFrame);
	const cv::Mat& mask = *data.get(a_inMask);

	if(mask.rows != input.rows || mask.cols != input.cols)
		throw std::runtime_error("Input and mask sizes have to match!");

	if(mask.channels() != 1 && mask.channels() != input.channels())
		throw std::runtime_error("The number of mask channels should be either 1 or the same as input image channels.");

	const float radius = data.get(a_radius);
	const int method = methodToEnum(data.get(a_method).value());

	cv::Mat result;

	// single mask mode - use all channels of the input image for inpainting
	if(mask.channels() == 1) {
		cv::inpaint(input, mask, result, radius, method);
	}

	// per-channel mode - use each channel of the mask for separate inpaining of each channel of the input
	else {
		// split the inputs and masks per channel
		std::vector<cv::Mat> inputs, masks;
		cv::split(input, inputs);
		cv::split(mask, masks);

		std::vector<cv::Mat> results(inputs.size());

		tbb::task_group tasks;

		for(std::size_t c = 0; c < inputs.size(); ++c)
			tasks.run([c, &inputs, &masks, &results, radius, method]() {
				cv::inpaint(inputs[c], masks.size() == 1 ? masks[0] : masks[c], results[c], radius, method);
			});

		tasks.wait();

		// and assemble the result
		cv::merge(results, result);
	}

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

}  // namespace
