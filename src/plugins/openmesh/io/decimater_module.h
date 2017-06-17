#pragma once

#include <dependency_graph/io/json.h>

#include "datatypes/decimater_module.h"

namespace std {

inline void to_json(::dependency_graph::io::json& j, const DecimaterModule& fn) {
	// not saved, ever
}

inline void from_json(const ::dependency_graph::io::json& j, DecimaterModule& fn) {
	// not loaded, ever
}

}
