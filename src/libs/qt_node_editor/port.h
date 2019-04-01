#pragma once

#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QSet>

namespace node_editor {

class Node;
class ConnectedEdge;

class Port : public QGraphicsRectItem {
	public:
		enum Type { kUnknown = 0, kInput = 1, kOutput = 2, kInputOutput = 3 };

		Port(const QString& name, Type t, QColor color, Node* parent, unsigned id);

		const QString name() const;
		const Type portType() const;
		const QColor color() const;
		const unsigned index() const;

		Node& parentNode();
		const Node& parentNode() const;

	private:
		float circleSize() const;

		static constexpr const float margin() {
			return 5;
		}

		void setWidth(unsigned w);
		unsigned minWidth() const;

		void adjustEdges();

		QGraphicsTextItem* m_name;

		QColor m_color;

		QGraphicsEllipseItem* m_in;
		QGraphicsEllipseItem* m_out;

		QSet<ConnectedEdge*> m_edges;
		Node* m_parent;
		unsigned m_id;

		friend class Node;
		friend class ConnectedEdge;
};

}
