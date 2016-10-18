#include "edge.h"

#include <cassert>
#include <iostream>

#include <QPainter>
#include <QGraphicsScene>

#include "port.h"

Edge::Edge(Port& p1, Port& p2) : m_p1(&p1), m_p2(&p2) {
	setFlags(ItemIsSelectable);

	assert(p1.portType() & Port::kOutput);
	assert(p2.portType() & Port::kInput);

	p1.m_edges.insert(this);
	p2.m_edges.insert(this);

	adjust();
}

void Edge::adjust() {
	const QRectF origBbox = boundingRect();

	const QRectF bb1 = m_p1->mapRectToScene(m_p1->boundingRect());
	const QRectF bb2 = m_p2->mapRectToScene(m_p2->boundingRect());

	const float x1 = bb1.x() + bb1.width();
	const float y1 = bb1.y() + bb1.height() / 2;
	const float x2 = bb2.x();
	const float y2 = bb2.y() + bb2.height() / 2;

	m_origin = QPointF(x1, y1);
	m_target = QPointF(x2, y2);

	if(scene())
		scene()->update(boundingRect().united(origBbox));
}

QRectF Edge::boundingRect() const {
	return QRectF(
	           QPointF(std::min(m_origin.x(), m_target.x()) - 1,
	                   std::min(m_origin.y(), m_target.y()) - 1),
	           QPointF(std::max(m_origin.x(), m_target.x()) + 1,
	                   std::max(m_origin.y(), m_target.y()) + 1)
	       );
}

void Edge::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	painter->drawLine(m_origin, m_target);
}
