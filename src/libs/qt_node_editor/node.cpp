#include "node.h"

#include <cassert>
#include <iostream>

#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QStyleOptionGraphicsItem>

#include "graph_scene.h"
#include "connected_edge.h"

namespace node_editor {

Node::Node(const QString& name, const QPointF& position, const QColor& color) : m_state(kOk), m_color(color) {
	setPos(position);
	setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
	setZValue(-1);

	m_titleBackground = new QGraphicsRectItem(this);
	m_title = new QGraphicsTextItem(name, this);


	m_titleBackground->setPen(Qt::NoPen);
	m_titleBackground->setBrush(QColor(m_color.red() + 32, m_color.green() + 32, m_color.blue() + 32));
	m_title->setDefaultTextColor(Qt::white);

	QFont font = m_title->font();
	font.setPixelSize(12);
	m_title->setFont(font);


	updateRect();
}

Node::~Node() {
	assert(scene());
	GraphScene* s = dynamic_cast<GraphScene*>(scene());
	assert(s);

	unsigned ei = 0;
	while(ei < s->edgeCount()) {
		ConnectedEdge& e = s->edge(ei);
		if((&(e.fromPort().parentNode()) == this || &(e.toPort().parentNode()) == this))
			s->disconnect(e);
		else
			++ei;
	}

	s->removeItem(this);
	s->remove(this);
}

const QString Node::name() const {
	return m_title->toPlainText();
}

void Node::setName(const QString& name) {
	m_title->setPlainText(name);
	updateRect();
}

void Node::updateRect() {
	prepareGeometryChange();

	unsigned height = m_title->boundingRect().height() + 4;
	unsigned width = m_title->boundingRect().width() + 4;

	for(auto& p : m_ports) {
		p->setPos(0, height);

		height += p->boundingRect().height();
		width = std::max(width, p->minWidth());
	}

	m_titleBackground->setRect(3, 3, width - 6, m_title->boundingRect().height() - 4);
	m_title->setPos((width - m_title->boundingRect().width()) / 2, 0);
	for(auto& p : m_ports)
		p->setRect(QRectF(p->rect().x(), p->rect().y(), width, p->rect().height()));

	setRect(QRectF(0, 0, width, height));
}

unsigned Node::portCount() const {
	return m_ports.size();
}

Port& Node::port(unsigned i) {
	assert(i < (unsigned)m_ports.size());
	return *m_ports[i];
}

void Node::addPort(const PortDefinition& def) {
	m_ports.push_back(new Port(def.name, def.type, def.color, this, m_ports.size()));

	updateRect();
}

void Node::removePort(Port& p) {
	auto it = std::find(m_ports.begin(), m_ports.end(), &p);
	assert(it != m_ports.end());

	delete *it;
	m_ports.erase(it);

	updateRect();
}


QVariant Node::itemChange(GraphicsItemChange change, const QVariant& value) {
	if(change == ItemPositionHasChanged && scene()) {
		// adjust the edges to the new node position
		for(auto& p : m_ports)
			p->adjustEdges();

		// and register that the node position has changed
		GraphScene* sc = dynamic_cast<GraphScene*>(scene());
		sc->registerNodeMove(this);
	}

	return value;
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	// paint as if not selected
	QStyleOptionGraphicsItem optionCopy(*option);
	optionCopy.state = optionCopy.state & (~QStyle::State_Selected);

	QPen pen;
	QBrush brush(m_color);
	switch(m_state) {
		case kOk:
			pen = QPen(QColor(m_color.red() + 32, m_color.green() + 32, m_color.blue() + 32), 3);
			break;
		case kInfo:
			pen = QPen(QColor(64, 64, 128), 3);
			break;
		case kWarning:
			pen = QPen(QColor(128, 128, 64), 3);
			brush.setColor(QColor(128, 128, 0));
			break;
		case kError:
			pen = QPen(QColor(255, 64, 64), 3);
			brush.setColor(QColor(192, 0, 0));
			break;
	}



	painter->setPen(pen);
	painter->setBrush(brush);

	if(option->state & QStyle::State_Selected)
		// and paint the white outline if selected
		painter->setPen(QPen(Qt::white, 3));

	QRectF rect(m_rect.left()+1, m_rect.top()+1, m_rect.width()-2, m_rect.height()-2);
	painter->drawRoundedRect(rect, 3, 3);
}

void Node::setState(const State& s) {
	m_state = s;

	update();
}

const Node::State& Node::state() const {
	return m_state;
}

QRectF Node::boundingRect() const {
	QRectF result = m_titleBackground->boundingRect();
	result |= m_rect;

	for(auto& p : m_ports)
		result |= p->mapToParent(p->boundingRect()).boundingRect();

	return result;
}

void Node::setRect(const QRectF& rect) {
	m_rect = rect;
}

}
