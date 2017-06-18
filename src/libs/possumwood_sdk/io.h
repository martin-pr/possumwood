#pragma once

#include <dependency_graph/io/json.h>

namespace possumwood {

template<typename T>
void to_json(::dependency_graph::io::json& j, const T& val) {
	val.toJson(j);
}

template<typename T>
void from_json(const ::dependency_graph::io::json& j, T& val) {
	val.fromJson(j);
}

}
