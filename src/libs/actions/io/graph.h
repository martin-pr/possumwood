#pragma once

#include <dependency_graph/network.h>
#include <dependency_graph/selection.h>

#include "json.h"

namespace possumwood { namespace io {

void to_json(json& j, const dependency_graph::Network& g, const dependency_graph::Selection& selection = dependency_graph::Selection());
void from_json(const json& j, dependency_graph::Network& g);

template<>
struct adl_serializer<dependency_graph::Network> {
	static void to_json(json& j, const dependency_graph::Network& g);
	static void from_json(const json& j, dependency_graph::Network& g);
};

} }
