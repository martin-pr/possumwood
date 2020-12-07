#include <possumwood_sdk/node_implementation.h>

namespace {

dependency_graph::InAttr<void> a_input, a_override;
dependency_graph::InAttr<bool> a_useOverride;
dependency_graph::OutAttr<void> a_output;

dependency_graph::State compute(dependency_graph::Values& values) {
	if(!values.get(a_useOverride))
		values.copy(a_input, a_output);
	else
		values.copy(a_override, a_output);

	return dependency_graph::State();
}  // namespace

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_input, "input");
	meta.addAttribute(a_override, "override");
	meta.addAttribute(a_useOverride, "use_override");
	meta.addAttribute(a_output, "output");

	meta.addInfluence(a_input, a_output);
	meta.addInfluence(a_override, a_output);
	meta.addInfluence(a_useOverride, a_output);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("override", init);

}  // namespace
