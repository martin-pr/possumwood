#include "node_base.h"

#include "graph.h"

namespace dependency_graph {

NodeBase::NodeBase(const std::string& name, const UniqueId& id, const MetadataHandle& metadata, Network* parent) : m_name(name), m_network(parent), m_index(id), m_metadata(metadata), m_data(metadata) {
	for(std::size_t a = 0; a < metadata.metadata().attributeCount(); ++a) {
		auto& meta = metadata.metadata().attr(a);
		assert(meta.offset() == a);

		m_ports.push_back(Port(meta.offset(), this));
	}
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

UniqueId NodeBase::index() const {
	return m_index;
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
			for(std::size_t i : metadata()->influences(p.index()))
				markAsDirty(i);
		}
		else {
			// all inputs connected to this output are marked dirty
			for(Port& o : network().connections().connectedTo(port(index)))
				o.node().markAsDirty(o.index());
		}
	}
}

const MetadataHandle& NodeBase::metadata() const {
	return m_metadata;
}

const Datablock& NodeBase::datablock() const {
	return m_data;
}

Datablock& NodeBase::datablock() {
	return m_data;
}

void NodeBase::setDatablock(const Datablock& data) {
	assert(data.meta() == metadata());
	m_data = data;
}

Port& NodeBase::port(size_t index) {
	assert(index < m_ports.size());
	return m_ports[index];
}

const Port& NodeBase::port(size_t index) const {
	assert(index < m_ports.size());
	return m_ports[index];
}

const size_t NodeBase::portCount() const {
	return m_ports.size();
}

}
