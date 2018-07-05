#pragma once

#include <dependency_graph/node.h>

#include "json.h"

namespace possumwood { namespace io {

template<>
struct adl_serializer<dependency_graph::NodeBase> {
	static void to_json(json& j, const dependency_graph::NodeBase& g);
	static void from_json(const json& j, dependency_graph::NodeBase& g);
};

} }
