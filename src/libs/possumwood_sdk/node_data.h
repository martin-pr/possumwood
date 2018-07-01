#pragma once

#include <QPoint>

#include "io.h"
#include "traits.h"

namespace possumwood {

class NodeData {
	public:
		struct Point {
			float x = 0.0f, y = 0.0f;

			bool operator == (const Point& p) const {
				return p.x == x && p.y == y;
			}

			bool operator != (const Point& p) const {
				return p.x != x || p.y != y;
			}

			Point operator + (const Point& p) const {
				return Point{x + p.x, y + p.y};
			}
		};

		NodeData(const Point& p = Point{0, 0}) : m_position(p) {
		}

		const Point& position() const {
			return m_position;
		}

		void setPosition(const Point& p) {
			m_position = p;
		}

		bool operator ==(const NodeData& d) const {
			return m_position == d.m_position;
		}

		bool operator !=(const NodeData& d) const {
			return m_position != d.m_position;
		}

	private:
		Point m_position;
};

template<>
struct Traits<NodeData> {
	static IO<NodeData> io;
};

std::ostream& operator << (std::ostream& out, const NodeData& d);

}
