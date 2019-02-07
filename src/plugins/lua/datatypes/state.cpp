#include "state.h"

namespace possumwood { namespace lua {

State::State() {
	// create a new raw state from Lua
	m_state = luaL_newstate();

	// connect the state to luabind library
	luabind::open(m_state);
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

}}
