#pragma once

#include "../network.h"
#include "../selection.h"

#include "json.h"

namespace dependency_graph { namespace io {

void to_json(json& j, const Network& g, const Selection& selection = Selection());
void from_json(const json& j, Network& g);

template<>
struct adl_serializer<Network> {
	static void to_json(json& j, const Network& g);
	static void from_json(const json& j, Network& g);
};

} }
