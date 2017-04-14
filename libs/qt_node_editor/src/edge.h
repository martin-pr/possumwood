#pragma once

#include <QGraphicsLineItem>
#include <QPen>

namespace node_editor {

class GraphWidget;

class Edge : public QGraphicsItem {
	public:
		Edge(QPointF origin, QPointF target);
		virtual ~Edge();

		virtual QRectF boundingRect() const override;
		virtual QPainterPath shape() const override;

		void setPen(QPen pen);
		const QPen& pen() const;

		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

	protected:
		void setPoints(QPointF origin, QPointF target);
		const QPointF& origin() const;
		const QPointF& target() const;

	private:
		QPointF bezierPoint(float t) const;
		QPainterPath makePath() const;

		QPointF m_origin, m_target;
		QPen m_pen;

		friend class GraphScene;
};

}
