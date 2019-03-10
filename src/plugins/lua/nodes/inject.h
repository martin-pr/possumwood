#pragma once

#include <possumwood_sdk/node_implementation.h>

#include "lua/datatypes/context.h"
#include "lua/datatypes/variable.inl"

namespace possumwood { namespace lua {

struct NullModule {
	static std::string name() {
		return "";
	}

	static void init(State& s) {
		// nothing
	}
};

template<typename T, typename HOLDER = T, typename MODULE = NullModule>
struct Inject {
	struct Params {
		dependency_graph::InAttr<std::string> a_name;
		dependency_graph::InAttr<T> a_value;
		dependency_graph::InAttr<possumwood::lua::Context> a_inContext;
		dependency_graph::OutAttr<possumwood::lua::Context> a_outContext;
	};

	static dependency_graph::State compute(dependency_graph::Values& data, const Params& params) {
		possumwood::lua::Context context = data.get(params.a_inContext);

		// instantiate a module instance, and try to use it for initialisation
		if(!MODULE::name().empty())
			context.addModule(MODULE::name(), MODULE::init);

		// inject the data, by instantiating a HOLDER type instance
		//   assuming assignment operator between HOLDER and T
		HOLDER holder = data.get(params.a_value);
		context.addVariable(possumwood::lua::Variable(data.get(params.a_name), holder));
		data.set(params.a_outContext, context);

		return dependency_graph::State();
	}

	static void init(possumwood::Metadata& meta) {
		static Params s_params;

		meta.addAttribute(s_params.a_name, "name", std::string("constant"));
		meta.addAttribute(s_params.a_value, "value", T());
		meta.addAttribute(s_params.a_inContext, "in_context");

		meta.addAttribute(s_params.a_outContext, "out_context");

		meta.addInfluence(s_params.a_name, s_params.a_outContext);
		meta.addInfluence(s_params.a_value, s_params.a_outContext);
		meta.addInfluence(s_params.a_inContext, s_params.a_outContext);

		meta.setCompute([](dependency_graph::Values& data) {
			return compute(data, s_params);
		});
	}
};

} }
