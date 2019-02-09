#include "state.h"

#include "context.h"

#include <lualib.h>

namespace possumwood { namespace lua {

State::State(const Context& con) {
	// create a new raw state from Lua
	m_state = luaL_newstate();

	// connect the state to luabind library
	luabind::open(m_state);

	// add all modules (libraries)
	for(auto& m : con.m_modules)
		m.second(*this);

	// add all variables from the context
	for(auto& v : con.m_variables)
		v->init(*this);
}

State::~State() {
	// release the Lua state
	lua_close(m_state);
}

luabind::object State::globals() const {
	return luabind::globals(m_state);
}

State::operator lua_State*() {
	return m_state;
}

State::operator const lua_State*() const {
	return m_state;
}

std::ostream& operator << (std::ostream& out, const std::shared_ptr<const State>& st) {
	out << "(state)";

	return out;
}

}}
