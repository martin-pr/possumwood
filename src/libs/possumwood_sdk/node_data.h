#pragma once

#include <QPoint>

#include "io.h"
#include <dependency_graph/unique_id.h>
#include "traits.h"

namespace possumwood {

class NodeData {
	public:
		NodeData(const QPointF& p = QPointF(0, 0)) : m_position(p) {
		}

		const QPointF& position() const {
			return m_position;
		}

		void setPosition(const QPointF& p) {
			m_position = p;
		}

		bool operator ==(const NodeData& d) const {
			return m_position == d.m_position;
		}

		bool operator !=(const NodeData& d) const {
			return m_position != d.m_position;
		}

	private:
		QPointF m_position;
};

template<>
struct Traits<NodeData> {
	static IO<NodeData> io;
};

}
