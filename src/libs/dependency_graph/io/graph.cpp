#include "graph.h"

#include "node.h"

namespace dependency_graph { namespace io {

void adl_serializer<Graph>::to_json(json& j, const ::dependency_graph::Graph& g) {
	j["nodes"] = "[]"_json;
	for(auto& n : g.nodes()) {
		j["nodes"].push_back("{}"_json);
		j["nodes"].back() = n;
	}

	j["connections"] = "[]"_json;
	for(auto& c : g.connections()) {
		j["connections"].push_back("{}"_json);

		auto& connection = j["connections"].back();
		connection["out_node"] = c.first.node().index();
		connection["out_port"] = c.first.index();
		connection["in_node"] = c.second.node().index();
		connection["in_port"] = c.second.index();
	}
}

void adl_serializer<Graph>::from_json(const json& j, ::dependency_graph::Graph& g) {
	// first, clear the graph
	g.nodes().clear();

	// TODO
}

} }
