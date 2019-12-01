#pragma once

#include <string>

#include <lua/datatypes/state.h>

namespace possumwood { namespace images {

struct Module {
	static std::string name();
	static void init(possumwood::lua::State& state);
};

} }
