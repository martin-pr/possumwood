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
	g.nodes().clear();

	for(auto& n : j["nodes"]) {
		Node& node = g.nodes().add(Metadata::instance(n["type"].get<std::string>()), n["name"].get<std::string>());
		adl_serializer<Node>::from_json(n, node);
	}

	for(auto& c : j["connections"]) {
		g.nodes()[c["out_node"].get<size_t>()].port(c["out_port"].get<size_t>()).connect(
			g.nodes()[c["in_node"].get<size_t>()].port(c["in_port"].get<size_t>()));
	}
}

} }
