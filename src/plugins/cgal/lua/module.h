#pragma once

#include <lua/datatypes/state.h>
#include <string>

namespace possumwood {
namespace cgal {

struct Module {
	static std::string name();
	static void init(possumwood::lua::State& state);
};

}  // namespace images
}  // namespace possumwood
