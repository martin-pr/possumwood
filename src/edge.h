#pragma once

#include <QGraphicsLineItem>

class Port;

class Edge : public QGraphicsItem {
	public:
		Edge(Port& p1, Port& p2);

		virtual QRectF boundingRect() const override;
		virtual QPainterPath shape() const override;
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

	private:
		void adjust();

		QPointF bezierPoint(float t) const;
		QPainterPath makePath() const;

		Port* m_p1, *m_p2;

		QPointF m_origin, m_target;

		friend class Port;
};
