#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

namespace {

dependency_graph::InAttr<void> a_data;

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_data, "data");

	meta.setCompute([](dependency_graph::Values& vals) {
		// default implementation does nothing

		return dependency_graph::State();
	});
}

possumwood::NodeImplementation s_impl("output", init);

}
