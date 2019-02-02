#include <exprtk/exprtk.hpp>

#include <memory>

#include <possumwood_sdk/node_implementation.h>

#include <luabind/luabind.hpp>

namespace {

dependency_graph::InAttr<std::string> a_src;
dependency_graph::OutAttr<float> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	const std::string& src = data.get(a_src);

	lua_State *myLuaState = luaL_newstate();

	luabind::open(myLuaState);
	luaL_dostring(myLuaState, src.c_str());

	const float out = luabind::call_function<float>(myLuaState, "main");
	data.set(a_out, out);

	lua_close(myLuaState);

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_src, "source", std::string("function main()\n  return 0\nend\n"));
	meta.addAttribute(a_out, "out");

	meta.addInfluence(a_src, a_out);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("lua/script", init);

}
