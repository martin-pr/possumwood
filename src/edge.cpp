#include "edge.h"

#include <cassert>
#include <iostream>

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
	const QRectF bb1 = m_p1->mapRectToScene(m_p1->boundingRect());
	const QRectF bb2 = m_p2->mapRectToScene(m_p2->boundingRect());

	setLine(bb1.x() + bb1.width(), bb1.y() + bb1.height() / 2,
	        bb2.x(), bb2.y() + bb2.height() / 2);
}
