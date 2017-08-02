#pragma once

#include <QPoint>

#include "io.h"
#include "unique_id.h"

namespace possumwood {

class NodeData {
	public:
		NodeData(const QPointF& p = QPointF(0, 0), const UniqueId& i = UniqueId()) : m_position(p), m_id(i) {
		}

		const QPointF& position() const {
			return m_position;
		}

		void setPosition(const QPointF& p) {
			m_position = p;
		}

		const UniqueId& id() const {
			return m_id;
		}

		bool operator ==(const NodeData& d) const {
			return m_position == d.m_position && m_id == d.m_id;
		}

		bool operator !=(const NodeData& d) const {
			return m_position != d.m_position || m_id != d.m_id;
		}

		void fromJson(const dependency_graph::io::json& json) {
			m_position.setX(json["x"].get<float>());
			m_position.setY(json["y"].get<float>());
		}

		void toJson(dependency_graph::io::json& json) const {
			json["x"] = m_position.x();
			json["y"] = m_position.y();
		}

	private:
		QPointF m_position;
		UniqueId m_id;
};

}
