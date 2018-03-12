#include "network.h"

namespace dependency_graph {

Network::Network(Graph* parent) : m_graph(parent), m_nodes(parent) {
}

bool Network::empty() const {
	return m_nodes.empty();
}

void Network::clear() {
	// clear only nodes - connections will clear themselves with the nodes
	m_nodes.clear();
}

Nodes& Network::nodes() {
	return m_nodes;
}

const Nodes& Network::nodes() const {
	return m_nodes;
}

Connections& Network::connections() {
	return m_connections;
}

const Connections& Network::connections() const {
	return m_connections;
}


}
