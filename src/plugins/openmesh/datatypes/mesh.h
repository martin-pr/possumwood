#pragma once

#include <possumwood_sdk/traits.h>

#include "openmesh.h"

namespace possumwood {

template<>
struct Traits<std::shared_ptr<const Mesh>> {

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 1, 1}};
	}
};

}
