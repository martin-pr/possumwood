#pragma once

#include <dependency_graph/io/json.h>

#include "openmesh.h"

namespace std {

inline void to_json(::dependency_graph::io::json& j, const std::shared_ptr<const Mesh>& fn) {
	// not saved, ever
}

inline void from_json(const ::dependency_graph::io::json& j, std::shared_ptr<const Mesh>& fn) {
	// not loaded, ever
}

}
