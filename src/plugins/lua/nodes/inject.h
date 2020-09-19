#pragma once

#include <possumwood_sdk/node_implementation.h>

#include "lua/datatypes/context.h"
#include "lua/datatypes/state.h"
#include "lua/datatypes/variable.inl"

namespace possumwood {
namespace lua {

struct NullModule {
	static std::string name() {
		return "";
	}

	static void init(::possumwood::lua::State& s) {
		// nothing
	}
};

template <typename T, typename HOLDER = T, typename MODULE = NullModule, AttrFlags FLAGS = AttrFlags(0)>
struct Inject {
	dependency_graph::InAttr<std::string> a_name;
	dependency_graph::InAttr<T> a_value;
	dependency_graph::InAttr<::possumwood::lua::Context> a_inContext;
	dependency_graph::OutAttr<::possumwood::lua::Context> a_outContext;

	dependency_graph::State compute(dependency_graph::Values& data) {
		::possumwood::lua::Context context = data.get(a_inContext);

		// instantiate a module instance, and try to use it for initialisation
		if(!MODULE::name().empty())
			context.addModule(MODULE::name(), MODULE::init);

		// inject the data, by instantiating a HOLDER type instance
		//   assuming assignment operator between HOLDER and T
		HOLDER holder = HOLDER(data.get(a_value));
		context.addVariable(::possumwood::lua::Variable(data.get(a_name), holder));
		data.set(a_outContext, context);

		return dependency_graph::State();
	}

	void init(::possumwood::Metadata& meta) {
		meta.addAttribute(a_name, "name", std::string("constant"));
		meta.addAttribute(a_value, "value", T(), FLAGS);
		meta.addAttribute(a_inContext, "in_context", ::possumwood::lua::Context(), ::possumwood::AttrFlags::kVertical);

		meta.addAttribute(a_outContext, "out_context", ::possumwood::lua::Context(),
		                  ::possumwood::AttrFlags::kVertical);

		meta.addInfluence(a_name, a_outContext);
		meta.addInfluence(a_value, a_outContext);
		meta.addInfluence(a_inContext, a_outContext);

		meta.setCompute([this](dependency_graph::Values& data) { return compute(data); });
	}
};

}  // namespace lua
}  // namespace possumwood
