#include "cgal/lua/module.h"

#include <lua/datatypes/context.h>
#include <lua/datatypes/state.h>

#include <possumwood_sdk/node_implementation.h>

namespace {

dependency_graph::InAttr<possumwood::lua::Context> a_inContext;
dependency_graph::OutAttr<possumwood::lua::Context> a_outContext;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::lua::Context context = data.get(a_inContext);

	context.addModule("cgal", possumwood::cgal::Module::init);

	data.set(a_outContext, context);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inContext, "in_context", possumwood::lua::Context(),
	                  possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outContext, "out_context", possumwood::lua::Context(),
	                  possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inContext, a_outContext);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl_opencv("lua/modules/cgal", init);

}  // namespace
