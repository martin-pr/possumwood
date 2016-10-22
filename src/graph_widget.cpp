#include "graph_widget.h"

#include <cassert>

#include <QHBoxLayout>
#include <QGLWidget>
#include <QResizeEvent>

#include "edge.h"

GraphWidget::GraphWidget(QWidget* parent) : QGraphicsView(parent) {
	m_scene = new QGraphicsScene(this);
	m_scene->setSceneRect(-width() / 2, -height() / 2, width(), height());

	setScene(m_scene);
	setDragMode(QGraphicsView::RubberBandDrag);
	setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	setBackgroundBrush(Qt::black);
}

void GraphWidget::clear() {
	m_scene->clear();
	m_nodes.clear();
}

Node& GraphWidget::node(unsigned index) {
	assert(index < (unsigned)m_nodes.size());
	return *m_nodes[index];
}

const Node& GraphWidget::node(unsigned index) const {
	assert(index < (unsigned)m_nodes.size());
	return *m_nodes[index];
}

unsigned GraphWidget::nodeCount() const {
	return m_nodes.size();
}

Edge& GraphWidget::edge(unsigned index) {
	assert(index < (unsigned)m_edges.size());
	return *m_edges[index];
}

const Edge& GraphWidget::edge(unsigned index) const {
	assert(index < (unsigned)m_edges.size());
	return *m_edges[index];
}

unsigned GraphWidget::edgeCount() const {
	return m_edges.size();
}

Node& GraphWidget::addNode(const QString& name,
                           const std::initializer_list<Node::PortDefinition>& ports) {

	return addNode(name, ports, mapToScene(mapFromGlobal(QCursor::pos())));
}

Node& GraphWidget::addNode(const QString& name,
                           const std::initializer_list<Node::PortDefinition>& ports,
                           const QPointF& position) {

	Node* n = new Node(name, position, ports);
	m_nodes.push_back(n);
	m_scene->addItem(n);

	return *n;
}

void GraphWidget::removeNode(Node& n) {
	{
		auto i = m_edges.begin();
		while(i != m_edges.end())
			if((&(*i)->fromPort().parentNode() == &n || &(*i)->toPort().parentNode() == &n)) {
				m_scene->removeItem(*i);
				i = m_edges.erase(i);
			}
			else
				++i;
	}

	{
		auto i = m_nodes.begin();
		while(i != m_nodes.end())
			if(*i == &n) {
				m_scene->removeItem(*i);
				i = m_nodes.erase(i);
			}
			else
				++i;
	}
}

void GraphWidget::connect(Port& p1, Port& p2) {
	if((p1.portType() & Port::kOutput) && (p2.portType() & Port::kInput)) {
		Edge* e = new Edge(p1, p2);
		m_edges.push_back(e);
		m_scene->addItem(e);
	}
}

void GraphWidget::disconnect(Port& p1, Port& p2) {
	auto it = m_edges.begin();
	while(it != m_edges.end())
		if((&(*it)->fromPort() == &p1) && (&(*it)->toPort() == &p2))
			it = m_edges.erase(it);
		else
			++it;
}

void GraphWidget::disconnect(Edge& e) {
	auto it = std::find(m_edges.begin(), m_edges.end(), &e);
	if(it != m_edges.end()) {
		m_scene->removeItem(*it);
		m_edges.erase(it);
	}
}

void GraphWidget::resizeEvent(QResizeEvent* event) {
	const QPointF mid = m_scene->sceneRect().center();

	m_scene->setSceneRect(mid.x() - event->size().width() / 2,
	                      mid.y() - event->size().height() / 2,
	                      event->size().width(),
	                      event->size().height());

	QGraphicsView::resizeEvent(event);
}

void GraphWidget::mousePressEvent(QMouseEvent* event) {
	if(event->button() == Qt::RightButton) {
		if(m_contextMenuCallback)
			m_contextMenuCallback(event->globalPos());
	}
	else
		QGraphicsView::mousePressEvent(event);
}

void GraphWidget::setContextMenuCallback(std::function<void(QPoint)> fn) {
	m_contextMenuCallback = fn;
}

void GraphWidget::setKeyPressCallback(std::function<void(const QKeyEvent&)> fn) {
	m_keyPressCallback = fn;
}

void GraphWidget::keyPressEvent(QKeyEvent* event) {
	if(m_keyPressCallback)
		m_keyPressCallback(*event);
}
