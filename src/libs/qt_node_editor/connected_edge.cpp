#include "connected_edge.h"

#include <cassert>
#include <iostream>

#include <QPainter>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>
#include <QPainterPathStroker>

#include "port.h"

namespace node_editor {

namespace {

QPointF pointFromPort(const Port& p, Port::Type t) {
	const QRectF bb = p.mapRectToScene(p.boundingRect());

	float x = 0;
	if(t == Port::Type::kInput)
		x = bb.left();
	else
		x = bb.right();
	const float y = bb.center().y();

	return QPoint(x, y);
}

}

ConnectedEdge::ConnectedEdge(Port& p1, Port& p2) :
	Edge(pointFromPort(p1, Port::Type::kOutput), pointFromPort(p2, Port::Type::kInput)),
	m_p1(&p1), m_p2(&p2) {

	setFlags(ItemIsSelectable);

	assert(p1.portType() == Port::Type::kOutput);
	assert(p2.portType() == Port::Type::kInput);

	p1.m_edges.insert(this);
	p2.m_edges.insert(this);

	QColor color;
	color.setRed(m_p1->color().red() / 2 + m_p2->color().red() / 2);
	color.setGreen(m_p1->color().green() / 2 + m_p2->color().green() / 2);
	color.setBlue(m_p1->color().blue() / 2 + m_p2->color().blue() / 2);

	setPen(QPen(color, 2));

	adjust();
}

ConnectedEdge::~ConnectedEdge() {
	{
		auto it = m_p1->m_edges.find(this);
		assert(it != m_p1->m_edges.end());
		m_p1->m_edges.erase(it);
	}

	{
		auto it = m_p2->m_edges.find(this);
		assert(it != m_p2->m_edges.end());
		m_p2->m_edges.erase(it);
	}
}

void ConnectedEdge::adjust() {
	const QRectF bb1 = m_p1->mapRectToScene(m_p1->boundingRect());
	const QRectF bb2 = m_p2->mapRectToScene(m_p2->boundingRect());

	const float x1 = bb1.right();
	const float y1 = bb1.center().y();
	const float x2 = bb2.left();
	const float y2 = bb2.center().y();

	setPoints(QPointF(x1, y1), QPointF(x2, y2));
}

const Port& ConnectedEdge::fromPort() const {
	assert(m_p1);
	return *m_p1;
}

const Port& ConnectedEdge::toPort() const {
	assert(m_p2);
	return *m_p2;
}

Port& ConnectedEdge::fromPort() {
	assert(m_p1);
	return *m_p1;
}

Port& ConnectedEdge::toPort() {
	assert(m_p2);
	return *m_p2;
}

}
