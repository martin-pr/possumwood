#pragma once

#include <dependency_graph/io/json.h>

#include "datatypes/filename.h"

inline void to_json(::dependency_graph::io::json& j, const Filename& fn) {
	j = fn.filename().string();
}

inline void from_json(const ::dependency_graph::io::json& j, Filename& fn) {
	fn.setFilename(j.get<std::string>());
}
