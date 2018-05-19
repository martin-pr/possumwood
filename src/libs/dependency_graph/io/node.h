#pragma once

#include "../node.h"

#include "json.h"

namespace dependency_graph { namespace io {

template<>
struct adl_serializer<NodeBase> {
	static void to_json(json& j, const NodeBase& g);
	static void from_json(const json& j, NodeBase& g);
};

} }
