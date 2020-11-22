#pragma once

#include <lua/datatypes/state.h>

#include <string>

namespace possumwood {
namespace lua {

struct MathsModule {
	static std::string name();
	static void init(possumwood::lua::State& state);
};

}  // namespace lua
}  // namespace possumwood
