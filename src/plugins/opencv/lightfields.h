#pragma once

#include <actions/traits.h>

#include <lightfields/metadata.h>
#include <lightfields/samples.h>
#include <lightfields/pattern.h>

namespace possumwood {

template<>
struct Traits<lightfields::Metadata> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.7, 0, 0}};
	}
};

template<>
struct Traits<lightfields::Samples> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.5, 0, 0}};
	}
};

template<>
struct Traits<::lightfields::Pattern> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0, 0}};
	}
};

}
