#include "port.h"

#include <iostream>
#include <cassert>

#include <QBrush>
#include <QPen>
#include <QFont>

#include "connected_edge.h"
#include "node.h"

namespace node_editor {

Port::Port(const QString& name, Port::Type t, QColor color, Node* parent, unsigned id) : QGraphicsRectItem(parent),
	m_color(color), m_in(NULL), m_out(NULL), m_parent(parent), m_id(id) {
	m_name = new QGraphicsTextItem(name, this);
	m_name->setPos(margin()+m_name->boundingRect().height()/2, 0);
	m_name->setDefaultTextColor(QColor(192, 192, 192));

	QFont font = m_name->font();
	font.setPixelSize(12);
	m_name->setFont(font);

	if(t & kInput) {
		m_in = new QGraphicsEllipseItem(
		    -circleSize()/2, margin(),
		    circleSize(), circleSize(),
		    this);
		m_in->setBrush(m_color);
		m_in->setPen(Qt::NoPen);
	}

	if(t & kOutput) {
		m_out = new QGraphicsEllipseItem(
		    minWidth() - circleSize()/2, margin(),
		    circleSize(), circleSize(),
		    this);
		m_out->setBrush(m_color);
		m_out->setPen(Qt::NoPen);
	}

	setRect(QRect(rect().left(), rect().top(), minWidth(), m_name->boundingRect().height()));

	setPen(Qt::NoPen);

	setZValue(1);
}

unsigned Port::minWidth() const {
	return m_name->boundingRect().width() + 2 * margin() + circleSize();
}

void Port::setWidth(unsigned w) {
	setRect(QRect(rect().x(), rect().y(), w, m_name->boundingRect().height()));

	if(m_in)
		m_name->setPos(margin() + circleSize()/2, 0);
	else
		m_name->setPos(w - m_name->boundingRect().width() - margin() - circleSize()/2, 0);

	if(m_out)
		m_out->setRect(w - circleSize()/2, margin(),
		               circleSize(), circleSize());

}

const QString Port::name() const {
	return m_name->toPlainText();
}

const unsigned Port::index() const {
	return m_id;
}

const Port::Type Port::portType() const {
	if(m_in && m_out)
		return kInputOutput;
	else if(m_in)
		return kInput;
	else
		return kOutput;
}

void Port::adjustEdges() {
	for(auto& e : m_edges)
		e->adjust();
}

const QColor Port::color() const {
	return m_color;
}

Node& Port::parentNode() {
	assert(m_parent);
	return *m_parent;
}

const Node& Port::parentNode() const {
	assert(m_parent);
	return *m_parent;
}

float Port::circleSize() const {
	return m_name->boundingRect().height() - 2 * margin();
}

}
