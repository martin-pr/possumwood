#include "port.h"

#include <iostream>
#include <cassert>

#include <QBrush>
#include <QPen>
#include <QFont>
#include <QPainter>

#include "connected_edge.h"
#include "node.h"

namespace node_editor {

Port::Port(const QString& name, Port::Type t, QColor color, Node* parent, unsigned id) : QGraphicsItem(parent),
	m_color(color), m_in(NULL), m_out(NULL), m_parent(parent), m_id(id) {
	m_name = new QGraphicsTextItem(name, this);
	m_name->setPos(margin()+m_name->boundingRect().height()/2, 0);
	m_name->setDefaultTextColor(QColor(192, 192, 192));
	m_name->setParentItem(this);

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

		m_in->setParentItem(this);
	}

	if(t & kOutput) {
		m_out = new QGraphicsEllipseItem(
		    minWidth() - circleSize()/2, margin(),
		    circleSize(), circleSize(),
		    this);
		m_out->setBrush(m_color);
		m_out->setPen(Qt::NoPen);

		m_out->setParentItem(this);
	}

	setRect(QRect(rect().left(), rect().top(), minWidth(), m_name->boundingRect().height()));

	setZValue(1);
}

unsigned Port::minWidth() const {
	return m_name->boundingRect().width() + 2 * margin() + circleSize();
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

QRectF Port::boundingRect() const {
	QRectF result = m_name->mapRectToParent(m_name->boundingRect());
	result |= rect();

	if(m_in)
		result |= m_in->mapRectToParent(m_in->boundingRect());

	if(m_out)
		result |= m_out->mapRectToParent(m_out->boundingRect());

	return result;
}

void Port::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
	// nothing

	// painter->setPen(QPen(Qt::red, 1, Qt::DotLine));
	// painter->setBrush(Qt::NoBrush);
	// painter->drawRect(boundingRect());
}

QRectF Port::rect() const {
	return m_rect;
}

void Port::setRect(const QRectF& rect) {
	prepareGeometryChange();

	m_rect = QRectF(rect.x(), rect.y(), std::max(rect.width(), m_name->boundingRect().width()), m_name->boundingRect().height());

	if(m_in)
		m_name->setPos(margin() + circleSize()/2, 0);
	else
		m_name->setPos(m_rect.width() - m_name->boundingRect().width() - margin() - circleSize()/2, 0);

	if(m_out)
		m_out->setRect(m_rect.width() - circleSize()/2, margin(),
		               circleSize(), circleSize());
}

}
