#pragma once

#include <possumwood_sdk/node_implementation.h>

#include "lua/datatypes/context.h"
#include "lua/datatypes/variable.inl"

namespace possumwood {
namespace lua {

template <typename T, typename HOLDER = T, AttrFlags FLAGS = AttrFlags(0)>
struct Extract {
	struct Params {
		dependency_graph::InAttr<std::string> a_name;
		dependency_graph::InAttr<possumwood::lua::State> a_state;
		dependency_graph::OutAttr<T> a_out;
	};

	static dependency_graph::State compute(dependency_graph::Values& data, Params& params) {
		const possumwood::lua::State& state = data.get(params.a_state);

		if(!state)
			throw std::runtime_error("Uninitialised state - cannot extract a value.");

		try {
			// get the result
			T value = T(luabind::object_cast<HOLDER>(state.globals()[data.get(params.a_name)]));
			// and push it to the output
			data.set(params.a_out, value);
		} catch(const luabind::error& err) {
			throw std::runtime_error(lua_tostring(err.state(), -1));
		}

		return dependency_graph::State();
	}

	static void init(possumwood::Metadata& meta) {
		static Params s_params;

		meta.addAttribute(s_params.a_name, "name", std::string("variable"));
		meta.addAttribute(s_params.a_state, "state", std::move(possumwood::lua::State()));

		meta.addAttribute(s_params.a_out, "out", T(), FLAGS);

		meta.addInfluence(s_params.a_name, s_params.a_out);
		meta.addInfluence(s_params.a_state, s_params.a_out);

		meta.setCompute([&](dependency_graph::Values& data) { return compute(data, s_params); });
	}
};

}  // namespace lua
}  // namespace possumwood
