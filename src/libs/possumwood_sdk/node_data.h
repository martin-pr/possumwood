#pragma once

#include <QPoint>

#include "io.h"
#include "unique_id.h"
#include "traits.h"

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

	private:
		QPointF m_position;
		UniqueId m_id;
};

template<>
struct Traits<NodeData> {
	static IO<NodeData> io;
};

}
