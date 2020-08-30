#pragma once

#include <actions/traits.h>

#include <boost/noncopyable.hpp>
#include <luabind/luabind.hpp>

namespace possumwood {
namespace lua {

class Context;

class State final : public boost::noncopyable {
  public:
	State();
	State(const Context& con);
	~State();

	State(const State&) = delete;
	State& operator=(const State&) = delete;

	State(State&&);
	State& operator=(State&&);

	luabind::object globals() const;

	operator lua_State*();
	operator const lua_State*() const;

  private:
	lua_State* m_state;
};

std::ostream& operator<<(std::ostream& out, const State& st);

}  // namespace lua

template <>
struct Traits<lua::State> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 0, 1}};
	}
};

}  // namespace possumwood
