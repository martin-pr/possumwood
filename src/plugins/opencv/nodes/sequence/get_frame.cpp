#include <actions/traits.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_sequence;
dependency_graph::InAttr<unsigned> a_index;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_frame;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& seq = data.get(a_sequence);

	if(seq.size() <= data.get(a_index))
		throw std::runtime_error("Invalid index requested.");

	data.set(a_frame, possumwood::opencv::Frame(**(seq.begin() + data.get(a_index))));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_sequence, "sequence");
	meta.addAttribute(a_index, "index", 0u);
	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_sequence, a_frame);
	meta.addInfluence(a_index, a_frame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/get_frame", init);

}  // namespace
