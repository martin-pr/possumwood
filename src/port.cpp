#include "port.h"

#include <iostream>
#include <cassert>

#include <QBrush>
#include <QPen>

#include "connected_edge.h"
#include "node.h"

namespace {
const unsigned s_margin = 5;
}

Port::Port(const QString& name, Port::Type t, QColor color, Node* parent) : QGraphicsRectItem(parent), m_color(color), m_in(NULL), m_out(NULL), m_parent(parent) {
	m_name = new QGraphicsTextItem(name, this);
	m_name->setPos(m_name->boundingRect().height(), 0);
	m_name->setDefaultTextColor(QColor(192, 192, 192));

	if(t & kInput) {
		m_in = new QGraphicsRectItem(
		    s_margin, s_margin,
		    m_name->boundingRect().height() - 2 * s_margin, m_name->boundingRect().height() - 2 * s_margin,
		    this);
		m_in->setBrush(m_color);
		m_in->setPen(Qt::NoPen);
	}

	if(t & kOutput) {
		m_out = new QGraphicsRectItem(
		    m_name->boundingRect().height() + m_name->boundingRect().width() + s_margin, s_margin,
		    m_name->boundingRect().height() - 2 * s_margin, m_name->boundingRect().height() - 2 * s_margin,
		    this);
		m_out->setBrush(m_color);
		m_out->setPen(Qt::NoPen);
	}

	setRect(QRect(rect().x(), rect().y(), minWidth(), m_name->boundingRect().height()));

	setPen(Qt::NoPen);
}

unsigned Port::minWidth() const {
	return m_name->boundingRect().width() + m_name->boundingRect().height() * 2;
}

void Port::setWidth(unsigned w) {
	setRect(QRect(rect().x(), rect().y(), w, m_name->boundingRect().height()));

	if(m_in)
		m_name->setPos(m_name->boundingRect().height(), 0);
	else
		m_name->setPos(w - m_name->boundingRect().height() - m_name->boundingRect().width(), 0);

	if(m_out)
		m_out->setRect(w - m_name->boundingRect().height() + s_margin, s_margin,
		               m_name->boundingRect().height() - 2 * s_margin, m_name->boundingRect().height() - 2 * s_margin);
}

const QString Port::name() const {
	return m_name->toPlainText();
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
