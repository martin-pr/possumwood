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

void Graph::Connections::add(Node::Port& src, Node::Port& dest) {
	if(src.category() != Attr::kOutput)
		throw std::runtime_error("Attempting to connect an input as the origin of a connection.");

	if(dest.category() != Attr::kInput)
		throw std::runtime_error("Attempting to connect an output as the target of a connection.");

	// make a new connection
	m_connections.left.insert(std::make_pair(&src, &dest));
}

std::vector<std::reference_wrapper<const Node>> Graph::Connections::connectedFrom(const Node::Port& p) const {
	if(p.category() != Attr::kOutput)
		throw std::runtime_error("Connected From request can be only run on output ports.");

	// ugly const casts, to keep const-correctness, ironically
	auto it1 = m_connections.left.lower_bound(const_cast<Node::Port*>(&p));
	auto it2 = m_connections.left.upper_bound(const_cast<Node::Port*>(&p));

	std::vector<std::reference_wrapper<const Node>> result;
	for(auto it = it1; it != it2; ++it)
		result.push_back(std::cref(it->second->node()));
	return result;
}

std::vector<std::reference_wrapper<const Node>> Graph::Connections::connectedTo(const Node::Port& p) const {
	if(p.category() != Attr::kInput)
		throw std::runtime_error("Connected To request can be only run on input ports.");

	// ugly const casts, to keep const-correctness, ironically
	auto it1 = m_connections.right.lower_bound(const_cast<Node::Port*>(&p));
	auto it2 = m_connections.right.upper_bound(const_cast<Node::Port*>(&p));

	std::vector<std::reference_wrapper<const Node>> result;
	for(auto it = it1; it != it2; ++it)
		result.push_back(std::cref(it->second->node()));
	return result;
}

std::vector<std::reference_wrapper<Node>> Graph::Connections::connectedFrom(Node::Port& p) {
	if(p.category() != Attr::kOutput)
		throw std::runtime_error("Connected From request can be only run on output ports.");

	// ugly const casts, to keep const-correctness, ironically
	auto it1 = m_connections.left.lower_bound(&p);
	auto it2 = m_connections.left.upper_bound(&p);

	std::vector<std::reference_wrapper<Node>> result;
	for(auto it = it1; it != it2; ++it)
		result.push_back(std::ref(it->second->node()));
	return result;
}

std::vector<std::reference_wrapper<Node>> Graph::Connections::connectedTo(Node::Port& p) {
	if(p.category() != Attr::kInput)
		throw std::runtime_error("Connected To request can be only run on input ports.");

	// ugly const casts, to keep const-correctness, ironically
	auto it1 = m_connections.right.lower_bound(&p);
	auto it2 = m_connections.right.upper_bound(&p);

	std::vector<std::reference_wrapper<Node>> result;
	for(auto it = it1; it != it2; ++it)
		result.push_back(std::ref(it->second->node()));
	return result;

}
