#include "node_implementation.h"

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>

namespace {

dependency_graph::InAttr<float> input1, input2;
dependency_graph::OutAttr<float> output;

void compute(dependency_graph::Values& data) {
	const float a = data.get(input1);
	const float b = data.get(input2);

	data.set(output, a + b);
}

void init(Metadata& meta) {
	meta.addAttribute(input1, "input_1");
	meta.addAttribute(input2, "input_2");
	meta.addAttribute(output, "output");

	meta.addInfluence(input1, output);
	meta.addInfluence(input2, output);

	meta.setCompute(compute);
}

NodeImplementation s_impl("addition", init);

}
