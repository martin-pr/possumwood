#pragma once

#include <actions/traits.h>

#include "generic_polymesh.h"

namespace possumwood {

template<>
struct Traits<polymesh::GenericPolymesh> {
	// static IO<std::shared_ptr<const Mesh>> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 0}};
	}
};

}
