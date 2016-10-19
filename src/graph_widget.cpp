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
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate /*BoundingRectViewportUpdate*/);
	setDragMode(QGraphicsView::RubberBandDrag);
	setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

void GraphWidget::clear() {
	m_scene->clear();
	m_nodes.clear();
}

Node& GraphWidget::node(unsigned index) {
	assert(index < (unsigned)m_nodes.size());
	return *m_nodes[index];
}

unsigned GraphWidget::nodeCount() const {
	return m_nodes.size();
}

Node& GraphWidget::addNode(const QString& name, const QPointF& position,
                           const std::initializer_list<Node::PortDefinition>& ports) {

	Node* n = new Node(name, position, ports);
	m_nodes.push_back(n);
	m_scene->addItem(n);

	return *n;
}

void GraphWidget::removeNode(Node& n) {
	for(auto i = m_nodes.begin(); i != m_nodes.end(); ++i)
		if(*i == &n) {
			m_scene->removeItem(*i);
			m_nodes.erase(i);
		}
}

void GraphWidget::connect(Port& p1, Port& p2) {
	if((p1.portType() & Port::kOutput) && (p2.portType() & Port::kInput)) {
		Edge* e = new Edge(p1, p2);
		m_edges.push_back(e);
		m_scene->addItem(e);
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
