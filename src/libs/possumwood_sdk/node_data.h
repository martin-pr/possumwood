#pragma once

#include <QPoint>

#include "io.h"

namespace possumwood {

struct NodeData {
	QPointF position;

	bool operator ==(const NodeData& d) const {
		return position == d.position;
	}

	bool operator !=(const NodeData& d) const {
		return position != d.position;
	}

	void fromJson(const dependency_graph::io::json& json) {
		position.setX(json["x"].get<float>());
		position.setY(json["y"].get<float>());
	}

	void toJson(dependency_graph::io::json& json) const {
		json["x"] = position.x();
		json["y"] = position.y();
	}
};

}
