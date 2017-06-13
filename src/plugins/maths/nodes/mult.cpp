#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

namespace {

dependency_graph::InAttr<float> input1, input2;
dependency_graph::OutAttr<float> output;

void compute(dependency_graph::Values& data) {
	const float a = data.get(input1);
	const float b = data.get(input2);

	data.set(output, a * b);
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(input1, "a");
	meta.addAttribute(input2, "b");
	meta.addAttribute(output, "out");

	meta.addInfluence(input1, output);
	meta.addInfluence(input2, output);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("maths/mult", init);

}
