#include <possumwood_sdk/node_implementation.h>

#include "../interval.h"

namespace {

dependency_graph::InAttr<possumwood::maths::Interval> input;
dependency_graph::OutAttr<float> output;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::maths::Interval in = data.get(input);
	data.set(output, in.value());

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(input, "interval");
	meta.addAttribute(output, "out");

	meta.addInfluence(input, output);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("maths/interval", init);

}
