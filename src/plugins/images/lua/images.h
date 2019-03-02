#pragma once

#include <lua/datatypes/state.h>

#include <luabind/operator.hpp>

#include "pixmap_wrapper.h"

namespace possumwood { namespace images {

template<typename PIXMAP>
void init(lua::State& state, const std::string typePrefix /*LDR, HDR and such*/) {
	using namespace luabind;

	module(state, "images")
	[
		class_<PixmapWrapper<PIXMAP>, std::shared_ptr<PixmapWrapper<PIXMAP>>>((typePrefix + "image").c_str())
			.def(luabind::constructor<std::size_t, std::size_t>())
			.def("setPixel", &PixmapWrapper<PIXMAP>::setPixel)
			.def("pixel", &PixmapWrapper<PIXMAP>::pixel)
			.def("width", &PixmapWrapper<PIXMAP>::width)
			.def("height", &PixmapWrapper<PIXMAP>::height)
	];
}

}}
