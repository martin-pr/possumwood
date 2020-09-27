#pragma once

#include <QGraphicsLineItem>
#include <QPen>

#include "port.h"

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

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;

  protected:
	void setPoints(QPointF origin, QPointF target);
	const QPointF& origin() const;
	const QPointF& target() const;

	void setDirection(Port::Orientation ori, Port::Orientation tgt);

  private:
	QPointF bezierPoint(float t) const;
	QPainterPath makePath() const;

	QPointF m_origin, m_target;
	Port::Orientation m_originDirection, m_targetDirection;
	QPen m_pen;

	friend class GraphScene;
};

}  // namespace node_editor
