#include "graph.h"

#include <algorithm>

namespace dependency_graph {

Graph::Graph() : m_nodes(this) {
	// connect the signals from owned classes
	m_connections.onConnect([this](Port& p1, Port& p2) {
		connected(p1, p2);
	});

	m_connections.onDisconnect([this](Port& p1, Port& p2) {
		disconnected(p1, p2);
	});
}

bool Graph::empty() const {
	return m_nodes.empty();
}

void Graph::clear() {
	// clear only nodes - connections will clear themselves with the nodes
	m_nodes.clear();
}

Nodes& Graph::nodes() {
	return m_nodes;
}

const Nodes& Graph::nodes() const {
	return m_nodes;
}

Connections& Graph::connections() {
	return m_connections;
}

const Connections& Graph::connections() const {
	return m_connections;
}

std::unique_ptr<Node> Graph::makeNode(const std::string& name, const Metadata* md) {
	return std::unique_ptr<Node>(new Node(name, md, this));
}

boost::signals2::connection Graph::onAddNode(std::function<void(Node&)> callback) {
	return m_onAddNode.connect(callback);
}

boost::signals2::connection Graph::onRemoveNode(std::function<void(Node&)> callback) {
	return m_onRemoveNode.connect(callback);
}


boost::signals2::connection Graph::onConnect(std::function<void(Port&, Port&)> callback) {
	return m_onConnect.connect(callback);
}

boost::signals2::connection Graph::onDisconnect(std::function<void(Port&, Port&)> callback) {
	return m_onDisconnect.connect(callback);
}

boost::signals2::connection Graph::onBlindDataChanged(std::function<void(Node&)> callback) {
	return m_onBlindDataChanged.connect(callback);
}

boost::signals2::connection Graph::onNameChanged(std::function<void(Node&)> callback) {
	return m_onNameChanged.connect(callback);
}

boost::signals2::connection Graph::onDirty(std::function<void()> callback) {
	return m_onDirty.connect(callback);
}

boost::signals2::connection Graph::onStateChanged(std::function<void(const Node&)> callback) {
	return m_onStateChanged.connect(callback);
}

void Graph::connected(Port& p1, Port& p2) {
	m_onConnect(p1, p2);
}

void Graph::disconnected(Port& p1, Port& p2) {
	m_onDisconnect(p1, p2);
}

void Graph::nameChanged(Node& node) {
	m_onNameChanged(node);
}
void Graph::stateChanged(Node& node) {
	m_onStateChanged(node);
}
void Graph::dirtyChanged() {
	m_onDirty();
}
void Graph::nodeAdded(Node& node) {
	m_onAddNode(node);
}
void Graph::nodeRemoved(Node& node) {
	m_onRemoveNode(node);
}
void Graph::blindDataChanged(Node& node) {
	m_onBlindDataChanged(node);
}

}
