#pragma once

#include <actions/traits.h>

#include "../meshes.h"

namespace possumwood {

template <>
struct Traits<Meshes> {
	// static IO<std::shared_ptr<const Mesh>> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0, 0}};
	}
};

template <>
struct Traits<CGALNefPolyhedron> {
	// static IO<std::shared_ptr<const Mesh>> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.5, 1, 0}};
	}
};

}  // namespace possumwood
