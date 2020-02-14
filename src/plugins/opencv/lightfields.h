#pragma once

#include <actions/traits.h>

#include <lightfields/metadata.h>

namespace possumwood {

template<>
struct Traits<lightfields::Metadata> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.7, 0, 0}};
	}
};

}
