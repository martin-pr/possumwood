#include "node_base.h"

#include "graph.h"

namespace dependency_graph {

NodeBase::NodeBase(const std::string& name, Network* parent) : m_name(name), m_network(parent) {
}

NodeBase::~NodeBase() {
}

const std::string& NodeBase::name() const {
	return m_name;
}

void NodeBase::setName(const std::string& name) {
	m_name = name;
	network().graph().nameChanged(*this);
}

const Network& NodeBase::network() const {
	return *m_network;
}

Network& NodeBase::network() {
	return *m_network;
}

size_t NodeBase::index() const {
	return network().nodes().findNodeIndex(*this);
}

}
