#pragma once

#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QSet>

namespace node_editor {

class Node;
class ConnectedEdge;

class Port : public QGraphicsItem {
	public:
		enum struct Type { kUnknown = 0, kInput = 1, kOutput = 2 };

		enum struct Orientation {
			kHorizontal = 0,
			kVertical
		};

		Port(const QString& name, Type t, Orientation o, QColor color, Node* parent, unsigned id);

		const QString name() const;
		Type portType() const;
		const QColor color() const;
		unsigned index() const;
		Orientation orientation() const;

		Node& parentNode();
		const Node& parentNode() const;

		virtual QRectF boundingRect() const override;
		QRectF rect() const;

		void setRect(const QRectF& rect);

		QPointF connectionPoint() const;

		float circleSize() const;

	private:
		virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) override;

		static constexpr float margin() {
			return 6;
		}

		qreal minWidth() const;

		void adjustEdges();

		QGraphicsTextItem* m_name;
		Orientation m_orientation;

		QRectF m_rect;
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
