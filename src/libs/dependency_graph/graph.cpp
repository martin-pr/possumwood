#include "graph.h"

#include <algorithm>

namespace dependency_graph {

Graph::Graph() : m_nodes(this), m_connections(this) {

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

boost::signals2::connection Graph::onLog(std::function<void(State::MessageType, const std::string&)> callback) {
	return m_onLog.connect(callback);
}

}
