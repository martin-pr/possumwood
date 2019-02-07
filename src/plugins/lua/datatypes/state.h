#pragma once

#include <boost/noncopyable.hpp>

#include <luabind/luabind.hpp>

namespace possumwood { namespace lua {

class State final : public boost::noncopyable {
	public:
		State();
		~State();

		luabind::object globals() const;

		operator lua_State*();
		operator const lua_State*() const;

	private:
		lua_State* m_state;
};

}}
