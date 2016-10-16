#include "node.h"

#include <QPainter>

Node::Node(const QString& name, const QPointF& position) : QGraphicsItem(NULL) {
	setPos(position);

	m_title = new QGraphicsTextItem(name, this);

	setFlags(ItemIsMovable | ItemIsSelectable);
}

QRectF Node::boundingRect() const {
	return m_title->boundingRect();
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	painter->fillRect(boundingRect(), Qt::lightGray);
}

void Node::setName(const QString& n) {
	m_title->setPlainText(n);
	update();
}

const QString Node::name() const {
	return m_title->toPlainText();
}
