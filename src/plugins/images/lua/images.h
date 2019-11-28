#pragma once

#include <lua/datatypes/state.h>

#include <luabind/operator.hpp>

namespace possumwood { namespace images {

template<typename PIXMAP, typename WRAPPER>
void init(lua::State& state, const std::string typesuffix /*none, _hdr and such*/) {
	using namespace luabind;

	module(state, "images")
	[
		class_<WRAPPER>(("image" + typesuffix).c_str())
			.def(luabind::constructor<std::size_t, std::size_t>())
			.def("setPixel", &WRAPPER::setPixel)
			.def("pixel", &WRAPPER::pixel)
			.def("width", &WRAPPER::width)
			.def("height", &WRAPPER::height)
	];
}

}}
