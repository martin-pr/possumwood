#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/task_group.h>

#include <opencv2/opencv.hpp>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence, a_inMask;
dependency_graph::InAttr<float> a_radius;
dependency_graph::InAttr<possumwood::Enum> a_method;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSequence;

int methodToEnum(const std::string& method) {
	if(method == "INPAINT_NS")
		return cv::INPAINT_NS;
	else if(method == "INPAINT_TELEA")
		return cv::INPAINT_TELEA;

	throw std::runtime_error("Unknown inpaint method " + method);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& input = data.get(a_inSequence);
	const possumwood::opencv::Sequence& mask = data.get(a_inMask);

	if(mask.meta().rows != input.meta().rows || mask.meta().cols != input.meta().cols)
		throw std::runtime_error("Input and mask sizes have to match!");

	if(mask.meta().channels > 1 && mask.meta().channels != input.meta().channels)
		throw std::runtime_error("The number of mask channels should be either 1 or the same as input image channels.");

	if(!possumwood::opencv::Sequence::hasMatchingKeys(input, mask))
		throw std::runtime_error("The input and mask indices should match.");

	const float radius = data.get(a_radius);
	const int method = methodToEnum(data.get(a_method).value());

	possumwood::opencv::Sequence result;

	tbb::task_group tasks;

	// single mask mode - use all channels of the input image for inpainting
	if(mask.meta().channels == 1) {
		for(auto it = input.begin(); it != input.end(); ++it) {
			tasks.run([it, &mask, &radius, &method, &result]() {
				cv::Mat m;

				cv::inpaint(it->second, mask[it->first], m, radius, method);

				result[it->first] = std::move(m);
			});
		}
	}

	// per-channel mode - use each channel of the mask for separate inpaining of each channel of the input
	else {
		for(auto it = input.begin(); it != input.end(); ++it) {
			tasks.run([it, &mask, &radius, &method, &result]() {
				// split the inputs and masks per channel
				std::vector<cv::Mat> inputs, masks;
				cv::split(it->second, inputs);
				cv::split(mask[it->first], masks);

				// initialise output as separate images (can't use resize because cv::Mat copy shares data and is not
				// const correct)
				std::vector<cv::Mat> results;
				for(std::size_t i = 0; i < inputs.size(); ++i)
					results.push_back(cv::Mat(it->second.rows, it->second.cols, it->second.depth()));

				// inpaint each individual channel separately
				for(std::size_t c = 0; c < inputs.size(); ++c)
					cv::inpaint(inputs[c], masks.size() == 1 ? masks[0] : masks[c], results[c], radius, method);

				// and assemble the result
				cv::Mat m;
				cv::merge(results, m);
				result[it->first] = std::move(m);
			});
		}
	}

	tasks.wait();

	data.set(a_outSequence, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "Sequence");
	meta.addAttribute(a_inMask, "mask");
	meta.addAttribute(a_radius, "radius", 5.0f);
	meta.addAttribute(a_method, "method", possumwood::Enum({"INPAINT_NS", "INPAINT_TELEA"}));
	meta.addAttribute(a_outSequence, "out_Sequence");

	meta.addInfluence(a_inSequence, a_outSequence);
	meta.addInfluence(a_inMask, a_outSequence);
	meta.addInfluence(a_radius, a_outSequence);
	meta.addInfluence(a_method, a_outSequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/inpaint", init);

}  // namespace
