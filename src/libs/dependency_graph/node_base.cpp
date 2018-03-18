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
	assert(m_network != nullptr);
	return *m_network;
}

Network& NodeBase::network() {
	assert(m_network != nullptr);
	return *m_network;
}

bool NodeBase::hasParentNetwork() const {
	return m_network != nullptr;
}

const Graph& NodeBase::graph() const {
	if(hasParentNetwork())
		return network().graph();

	// TERRIBLE hacking, to be replaced with something more sensible at some point
	const Graph& g = dynamic_cast<const Graph&>(*this);
	return g;
}

Graph& NodeBase::graph() {
	if(hasParentNetwork())
		return network().graph();

	// TERRIBLE hacking, to be replaced with something more sensible at some point
	Graph& g = dynamic_cast<Graph&>(*this);
	return g;
}

size_t NodeBase::index() const {
	return network().nodes().findNodeIndex(*this);
}

void NodeBase::markAsDirty(size_t index) {
	Port& p = port(index);

	// mark the port itself as dirty
	if(!p.isDirty()) {
		p.setDirty(true);

		graph().dirtyChanged();

		// recurse + handle each port type slightly differently
		if(p.category() == Attr::kInput) {
			// all outputs influenced by this input are marked dirty
			for(std::size_t i : metadata().metadata().influences(p.index()))
				markAsDirty(i);
		}
		else {
			// all inputs connected to this output are marked dirty
			for(Port& o : network().connections().connectedTo(port(index)))
				o.node().markAsDirty(o.index());
		}
	}
}

}
