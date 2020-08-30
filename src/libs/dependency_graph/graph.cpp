#include "graph.h"

#include <algorithm>

#include "nodes.inl"

namespace dependency_graph {

struct Graph::Signals {
	boost::signals2::signal<void(NodeBase&)> m_onAddNode, m_onRemoveNode;
	boost::signals2::signal<void(NodeBase&)> m_onNameChanged, m_onBlindDataChanged, m_onMetadataChanged;
	boost::signals2::signal<void(Port&, Port&)> m_onConnect, m_onDisconnect;
	boost::signals2::signal<void()> m_onDirty;
	boost::signals2::signal<void(const NodeBase&)> m_onStateChanged;
};

Graph::Graph() : Network("network", UniqueId(), Network::defaultMetadata(), nullptr), m_signals(new Signals) {
}

Graph::~Graph() {
	clear();
}

boost::signals2::connection Graph::onAddNode(std::function<void(NodeBase&)> callback) {
	return m_signals->m_onAddNode.connect(callback);
}

boost::signals2::connection Graph::onRemoveNode(std::function<void(NodeBase&)> callback) {
	return m_signals->m_onRemoveNode.connect(callback);
}

boost::signals2::connection Graph::onConnect(std::function<void(Port&, Port&)> callback) {
	return m_signals->m_onConnect.connect(callback);
}

boost::signals2::connection Graph::onDisconnect(std::function<void(Port&, Port&)> callback) {
	return m_signals->m_onDisconnect.connect(callback);
}

boost::signals2::connection Graph::onBlindDataChanged(std::function<void(NodeBase&)> callback) {
	return m_signals->m_onBlindDataChanged.connect(callback);
}

boost::signals2::connection Graph::onNameChanged(std::function<void(NodeBase&)> callback) {
	return m_signals->m_onNameChanged.connect(callback);
}

boost::signals2::connection Graph::onDirty(std::function<void()> callback) {
	return m_signals->m_onDirty.connect(callback);
}

boost::signals2::connection Graph::onStateChanged(std::function<void(const NodeBase&)> callback) {
	return m_signals->m_onStateChanged.connect(callback);
}

boost::signals2::connection Graph::onMetadataChanged(std::function<void(NodeBase&)> callback) {
	return m_signals->m_onMetadataChanged.connect(callback);
}

void Graph::connected(Port& p1, Port& p2) {
	m_signals->m_onConnect(p1, p2);
}

void Graph::disconnected(Port& p1, Port& p2) {
	m_signals->m_onDisconnect(p1, p2);
}

void Graph::nameChanged(NodeBase& node) {
	m_signals->m_onNameChanged(node);
}

void Graph::stateChanged(NodeBase& node) {
	m_signals->m_onStateChanged(node);
}

void Graph::metadataChanged(NodeBase& node) {
	m_signals->m_onMetadataChanged(node);
}

void Graph::dirtyChanged() {
	m_signals->m_onDirty();
}

void Graph::nodeAdded(NodeBase& node) {
	m_signals->m_onAddNode(node);
}

void Graph::nodeRemoved(NodeBase& node) {
	m_signals->m_onRemoveNode(node);
}

void Graph::blindDataChanged(NodeBase& node) {
	m_signals->m_onBlindDataChanged(node);
}

}  // namespace dependency_graph
