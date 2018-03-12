#include "node_base.h"

#include "graph.h"

namespace dependency_graph {

NodeBase::NodeBase(const std::string& name, Graph* parent) : m_name(name), m_graph(parent) {
}

NodeBase::~NodeBase() {
}

const std::string& NodeBase::name() const {
	return m_name;
}

void NodeBase::setName(const std::string& name) {
	m_name = name;
	graph().nameChanged(*this);
}

const Graph& NodeBase::graph() const {
	return *m_graph;
}

Graph& NodeBase::graph() {
	return *m_graph;
}

size_t NodeBase::index() const {
	return graph().network().nodes().findNodeIndex(*this);
}

}
