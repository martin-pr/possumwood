#pragma once

#include <actions/traits.h>

#include "../selection.h"

namespace possumwood {

template <>
struct Traits<FaceSelection> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.5, 0.5}};
	}
};

}  // namespace possumwood
