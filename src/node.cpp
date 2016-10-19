#include "node.h"

#include <cassert>
#include <iostream>

#include <QPainter>
#include <QBrush>
#include <QPen>

Node::Node(const QString& name, const QPointF& position, const std::initializer_list<PortDefinition>& ports) {
	setPos(position);
	setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
	setZValue(-1);

	m_titleBackground = new QGraphicsRectItem(this);
	m_title = new QGraphicsTextItem(name, this);

	unsigned height = m_title->boundingRect().height();
	for(auto& p : ports) {
		m_ports.push_back(new Port(p.name, p.type, p.color, this));
		m_ports.back()->setPos(0, height);

		height += m_ports.back()->boundingRect().height();
	}

	updateRect();

	setBrush(Qt::lightGray);
	m_titleBackground->setPen(Qt::NoPen);
	m_titleBackground->setBrush(Qt::darkGray);
	m_title->setDefaultTextColor(Qt::white);
	setPen(Qt::NoPen);
}

const QString Node::name() const {
	return m_title->toPlainText();
}

void Node::updateRect() {
	unsigned height = m_title->boundingRect().height();
	unsigned width = m_title->boundingRect().width();

	for(auto& p : m_ports) {
		height += p->boundingRect().height();
		width = std::max(width, p->minWidth());
	}

	m_titleBackground->setRect(1, 1, width - 2, m_title->boundingRect().height() - 2);
	m_title->setPos((width - m_title->boundingRect().width()) / 2, 0);
	for(auto& p : m_ports)
		p->setWidth(width);

	setRect(0, 0, width, height);
}

unsigned Node::portCount() const {
	return m_ports.size();
}

Port& Node::port(unsigned i) {
	assert(i < (unsigned)m_ports.size());
	return *m_ports[i];
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant& value) {
	if(change == ItemPositionHasChanged && scene())
		for(auto& p : m_ports)
			p->adjustEdges();

	return value;
}
