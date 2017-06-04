#pragma once

#include "../node.h"

#include "json.h"

namespace dependency_graph { namespace io {

template<>
struct adl_serializer<Node> {
	static void to_json(json& j, const Node& g);
	static void from_json(const json& j, Node& g);
};

} }
