#pragma once

#include <actions/traits.h>

#include "lightfields/pattern.h"

namespace possumwood {

template<>
struct Traits<::lightfields::Pattern> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0, 0}};
	}
};

}
