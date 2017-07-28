#pragma once

#include <QPoint>

#include "io.h"
#include "unique_id.h"

namespace possumwood {

struct NodeData {
	QPointF position;
	UniqueId id;

	bool operator ==(const NodeData& d) const {
		return position == d.position && id == d.id;
	}

	bool operator !=(const NodeData& d) const {
		return position != d.position || id != d.id;
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
