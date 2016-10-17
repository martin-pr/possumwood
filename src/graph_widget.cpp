#include "graph_widget.h"

#include <cassert>

#include <QHBoxLayout>

GraphWidget::GraphWidget(QWidget* parent) : QWidget(parent) {
	m_scene = new QGraphicsScene(this);
	m_view = new QGraphicsView(m_scene, this);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_view);
}

void GraphWidget::clear() {
	m_scene->clear();
	m_nodes.clear();
}

const Node& GraphWidget::node(unsigned index) const {
	assert(index < (unsigned)m_nodes.size());
	return *m_nodes[index];
}

unsigned GraphWidget::nodeCount() const {
	return m_nodes.size();
}

const Node& GraphWidget::addNode(const QString& name, const QPointF& position,
                                 const std::initializer_list<std::pair<QString, Port::Type>>& ports) {

	Node* n = new Node(name, position, ports);
	m_nodes.push_back(n);
	m_scene->addItem(n);

	return *n;
}

void GraphWidget::removeNode(const Node& n) {
	Node* removed = NULL;
	for(auto i = m_nodes.begin(); i != m_nodes.end(); ++i)
		if(*i == &n) {
			removed = *i;
			m_nodes.erase(i);
		}

	m_scene->removeItem(removed);
}
