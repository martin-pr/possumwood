#include "graph_widget.h"

#include <cassert>

#include <QHBoxLayout>
#include <QGLWidget>

#include "edge.h"

GraphWidget::GraphWidget(QWidget* parent) : QWidget(parent) {
	m_scene = new QGraphicsScene(this);
	m_view = new QGraphicsView(m_scene, this);

	m_view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	m_view->setViewport(new QGLWidget());

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_view);
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
                           const std::initializer_list<std::pair<QString, Port::Type>>& ports) {

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

