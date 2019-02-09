#include <possumwood_sdk/node_implementation.h>

#include "datatypes/context.h"
#include "datatypes/variable.inl"

namespace {

dependency_graph::InAttr<std::string> a_name;
dependency_graph::InAttr<std::shared_ptr<const possumwood::lua::State>> a_state;
dependency_graph::OutAttr<float> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::shared_ptr<const possumwood::lua::State> state = data.get(a_state);

	if(!state)
		throw std::runtime_error("Uninitialised state - cannot extract a value.");

	try {
		// get the result
		float value = luabind::object_cast<float>(state->globals()[data.get(a_name)]);
		// and push it to the output
		data.set(a_out, value);
	}
	catch(const luabind::error& err) {
		throw std::runtime_error(lua_tostring(err.state(), -1));
	}

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_name, "name", std::string("variable"));
	meta.addAttribute(a_state, "state");

	meta.addAttribute(a_out, "out");

	meta.addInfluence(a_name, a_out);
	meta.addInfluence(a_state, a_out);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("lua/extract/float", init);

}
