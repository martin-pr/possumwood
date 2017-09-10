#pragma once

#include <vector>

#include <possumwood_sdk/traits.h>

#include "skeleton.h"

namespace anim {

/// a placeholder structure for storing keyframed animation (until a proper one is implemented)
struct Animation {
	std::vector<Skeleton> frames;
	float fps;
};

}

namespace possumwood {

template<>
struct Traits<std::shared_ptr<const anim::Animation>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 1.0, 0}};
	}
};

}
