#include "node_data.h"

void to_json(::dependency_graph::io::json& j, const NodeData& d) {
	j["x"] = d.position.x();
	j["y"] = d.position.y();
}

void from_json(const ::dependency_graph::io::json& j, NodeData& d) {
	d.position.setX(j["x"].get<float>());
	d.position.setY(j["y"].get<float>());
}

