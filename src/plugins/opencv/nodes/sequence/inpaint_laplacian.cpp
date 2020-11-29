#include <possumwood_sdk/node_implementation.h>

#include <mutex>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "laplacian_inpainting.h"
#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSeq, a_inMask;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSeq;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State state;

	const possumwood::opencv::Sequence& inSeq = data.get(a_inSeq);
	const possumwood::opencv::Sequence& maskSeq = data.get(a_inMask);

	if(!possumwood::opencv::Sequence::hasMatchingKeys(inSeq, maskSeq))
		throw std::runtime_error("Input and mask sequence indices do not match!");

	std::vector<cv::Mat> inputs;
	for(auto& i : inSeq)
		inputs.push_back(i.second);

	std::vector<cv::Mat> masks;
	for(auto& m : maskSeq)
		masks.push_back(m.second);

	std::vector<cv::Mat> result;

	state.append(possumwood::opencv::inpaint(inputs, masks, result));

	possumwood::opencv::Sequence out;

	auto it = result.begin();
	for(auto& i : inSeq)
		out[i.first] = std::move(*(it++));

	data.set(a_outSeq, out);

	return state;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSeq, "sequence");
	meta.addAttribute(a_inMask, "mask");
	meta.addAttribute(a_outSeq, "out_sequence");

	meta.addInfluence(a_inSeq, a_outSeq);
	meta.addInfluence(a_inMask, a_outSeq);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/inpaint_laplacian", init);

}  // namespace
