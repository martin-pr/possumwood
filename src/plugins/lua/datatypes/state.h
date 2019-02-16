#pragma once

#include <boost/noncopyable.hpp>

#include <luabind/luabind.hpp>

#include <actions/traits.h>

namespace possumwood { namespace lua {

class Context;

class State final : public boost::noncopyable {
	public:
		State(const Context& con);
		~State();

		luabind::object globals() const;

		operator lua_State*();
		operator const lua_State*() const;

	private:
		lua_State* m_state;
};

std::ostream& operator << (std::ostream& out, const std::shared_ptr<const State>& st);

}

template<>
struct Traits< std::shared_ptr<const lua::State> > {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 0, 1}};
	}
};

}
