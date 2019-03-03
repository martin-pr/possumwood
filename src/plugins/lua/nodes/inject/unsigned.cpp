#include <possumwood_sdk/node_implementation.h>

#include "datatypes/context.h"
#include "datatypes/variable.inl"

namespace {

dependency_graph::InAttr<std::string> a_name;
dependency_graph::InAttr<unsigned> a_value;
dependency_graph::InAttr<possumwood::lua::Context> a_inContext;
dependency_graph::OutAttr<possumwood::lua::Context> a_outContext;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::lua::Context context = data.get(a_inContext);
	context.addVariable(possumwood::lua::Variable(data.get(a_name), data.get(a_value)));
	data.set(a_outContext, context);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_name, "name", std::string("constant"));
	meta.addAttribute(a_value, "value", 0u);
	meta.addAttribute(a_inContext, "in_context");

	meta.addAttribute(a_outContext, "out_context");

	meta.addInfluence(a_name, a_outContext);
	meta.addInfluence(a_value, a_outContext);
	meta.addInfluence(a_inContext, a_outContext);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("lua/inject/unsigned", init);

}
