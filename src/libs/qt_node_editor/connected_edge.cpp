#include "connected_edge.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QPainterPathStroker>
#include <QStyleOptionGraphicsItem>
#include <cassert>
#include <iostream>

#include "port.h"

namespace node_editor {

ConnectedEdge::ConnectedEdge(Port& p1, Port& p2)
    : Edge(p1.connectionPoint(), p2.connectionPoint()), m_p1(&p1), m_p2(&p2) {
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
	setPoints(m_p1->connectionPoint(), m_p2->connectionPoint());
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

}  // namespace node_editor
