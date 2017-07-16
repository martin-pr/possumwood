#pragma once

#include <vector>

#include "skeleton.h"

namespace anim {
	
/// a placeholder structure for storing keyframed animation (until a proper one is implemented)
struct Animation {
	Skeleton base;
	std::vector<Skeleton> frames;
	float fps;
};

};