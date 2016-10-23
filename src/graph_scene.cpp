#include "graph_scene.h"

#include <cassert>

#include <QGraphicsView>

#include "node.h"
#include "connected_edge.h"

GraphScene::GraphScene(QGraphicsView* parent) : QGraphicsScene(parent) {

}

Node& GraphScene::node(unsigned index) {
	assert(index < (unsigned)m_nodes.size());
	return *m_nodes[index];
}

const Node& GraphScene::node(unsigned index) const {
	assert(index < (unsigned)m_nodes.size());
	return *m_nodes[index];
}

unsigned GraphScene::nodeCount() const {
	return m_nodes.size();
}

ConnectedEdge& GraphScene::edge(unsigned index) {
	assert(index < (unsigned)m_edges.size());
	return *m_edges[index];
}

const ConnectedEdge& GraphScene::edge(unsigned index) const {
	assert(index < (unsigned)m_edges.size());
	return *m_edges[index];
}

unsigned GraphScene::edgeCount() const {
	return m_edges.size();
}

// Node& GraphScene::addNode(const QString& name,
//                            const std::initializer_list<Node::PortDefinition>& ports) {

// 	return addNode(name, ports, mapToScene(mapFromGlobal(QCursor::pos())));
// }

Node& GraphScene::addNode(const QString& name,
                           const std::initializer_list<Node::PortDefinition>& ports,
                           const QPointF& position) {

	Node* n = new Node(name, position, ports);
	m_nodes.push_back(n);
	addItem(n);

	return *n;
}

void GraphScene::removeNode(Node& n) {
	{
		auto i = m_edges.begin();
		while(i != m_edges.end())
			if((&(*i)->fromPort().parentNode() == &n || &(*i)->toPort().parentNode() == &n)) {
				removeItem(*i);
				delete *i;
				i = m_edges.erase(i);
			}
			else
				++i;
	}

	{
		auto i = m_nodes.begin();
		while(i != m_nodes.end())
			if(*i == &n) {
				removeItem(*i);
				delete *i;
				i = m_nodes.erase(i);
			}
			else
				++i;
	}
}

void GraphScene::connect(Port& p1, Port& p2) {
	if((p1.portType() & Port::kOutput) && (p2.portType() & Port::kInput)) {
		ConnectedEdge* e = new ConnectedEdge(p1, p2);
		m_edges.push_back(e);
		addItem(e);
	}
}

void GraphScene::disconnect(Port& p1, Port& p2) {
	auto it = m_edges.begin();
	while(it != m_edges.end())
		if((&(*it)->fromPort() == &p1) && (&(*it)->toPort() == &p2)) {
			removeItem(*it);
			delete *it;
			it = m_edges.erase(it);
		}
		else
			++it;
}

void GraphScene::disconnect(ConnectedEdge& e) {
	auto it = std::find(m_edges.begin(), m_edges.end(), &e);
	if(it != m_edges.end()) {
		removeItem(*it);
		delete *it;
		m_edges.erase(it);
	}
}

