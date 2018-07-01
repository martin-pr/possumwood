#include "node_data.h"

namespace possumwood {

namespace {

void toJson(::dependency_graph::io::json& json, const NodeData& value) {
	json["x"] = value.position().x();
	json["y"] = value.position().y();
}

void fromJson(const ::dependency_graph::io::json& json, NodeData& value) {
	value.setPosition(QPointF(
		json["x"].get<float>(),
		json["y"].get<float>()
	));
}

}

IO<NodeData> Traits<NodeData>::io(&toJson, &fromJson);

std::ostream& operator << (std::ostream& out, const NodeData& d) {
	out << "(node data)";

	return out;
}

}
