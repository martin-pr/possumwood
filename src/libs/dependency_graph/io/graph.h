#pragma once

#include "../graph.h"
#include "../selection.h"

#include "json.h"

namespace dependency_graph { namespace io {

template<>
struct adl_serializer<Graph> {
	static void to_json(json& j, const Graph& g, const Selection& selection = Selection());
	static void from_json(const json& j, Graph& g);
};

} }
