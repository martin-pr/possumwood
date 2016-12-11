#include "graph.h"

#include <algorithm>

Graph::Graph() : m_nodes(this) {

}

Graph::Nodes& Graph::nodes() {
	return m_nodes;
}

const Graph::Nodes& Graph::nodes() const {
	return m_nodes;
}

Graph::Connections& Graph::connections() {
	return m_connections;
}

const Graph::Connections& Graph::connections() const {
	return m_connections;
}

std::unique_ptr<Node> Graph::makeNode(const std::string& name, const Metadata& md) {
	return std::move(std::unique_ptr<Node>(new Node(name, md, this)));
}

Graph::Nodes::Nodes(Graph* parent) : m_parent(parent) {

}

Node& Graph::Nodes::add(const Metadata& type, const std::string& name) {
	m_nodes.push_back(std::move(m_parent->makeNode(name, type)));
	return *m_nodes.back();
}

Graph::Connection& Graph::Connections::add(const Node::Port& src, const Node::Port& dest) {
	// try to find existing connection with this src and dest
	auto it = std::find(m_connections.begin(), m_connections.end(), Connection{&src, &dest});
	// return existing connection
	if(it != m_connections.end())
		return *it;

	// make a new connection
	else {
		m_connections.push_back(Connection{&src, &dest});
		return *(m_connections.begin() + (m_connections.size()-1));
	}
}
