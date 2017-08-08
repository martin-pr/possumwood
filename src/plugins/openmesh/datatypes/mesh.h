#pragma once

#include <possumwood_sdk/traits.h>

#include "openmesh.h"

namespace possumwood {

template<>
struct Traits<std::shared_ptr<const Mesh>> {
	static IO<std::shared_ptr<const Mesh>> io;
};

}
