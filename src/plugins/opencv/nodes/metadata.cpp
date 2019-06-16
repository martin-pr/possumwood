#include <memory>

#include <possumwood_sdk/node_implementation.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_frame;
dependency_graph::OutAttr<unsigned> a_width, a_height;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Frame& input = data.get(a_frame);

	data.set(a_width, (unsigned)(*input).cols);
	data.set(a_height, (unsigned)(*input).rows);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_width, "width", 0u);
	meta.addAttribute(a_height, "height", 0u);

	meta.addAttribute(a_frame, "frame");

	meta.addInfluence(a_frame, a_width);
	meta.addInfluence(a_frame, a_height);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/metadata", init);

}
