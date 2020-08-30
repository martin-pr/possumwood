#include "node_data.h"

namespace possumwood {

namespace {

void toJson(::possumwood::io::json& json, const NodeData& value) {
	json["x"] = value.position().x;
	json["y"] = value.position().y;
}

void fromJson(const ::possumwood::io::json& json, NodeData& value) {
	value.setPosition(NodeData::Point{json["x"].get<float>(), json["y"].get<float>()});
}

}  // namespace

IO<NodeData> Traits<NodeData>::io(&toJson, &fromJson);

std::ostream& operator<<(std::ostream& out, const NodeData& d) {
	out << "(node data)";

	return out;
}

}  // namespace possumwood
