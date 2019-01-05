#include "network.h"

#include "graph.h"
#include "metadata_register.h"

namespace dependency_graph {

Network::Network(const std::string& name, const UniqueId& id, const MetadataHandle& md, Network* parent) : NodeBase(name, id, md, parent), m_nodes(this), m_connections(this) {

}

Network::~Network() {
	clear();
}

bool Network::empty() const {
	return m_nodes.empty() && m_connections.empty();
}

void Network::clear() {
	// first, unlink all the ports
	for(unsigned i=0; i<portCount(); ++i) {
		Port& p = port(i);
		if(p.isLinked())
			p.unlink();
	}

	// clear all subnetworks
	for(auto& n : m_nodes)
		if(n.is<Network>())
			n.as<Network>().clear();

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

std::unique_ptr<NodeBase> Network::makeNode(const std::string& name, const MetadataHandle& md, const UniqueId& id) {
	return md->createNode(name, *this, id);
}

const MetadataHandle& Network::defaultMetadata() {
	static std::unique_ptr<MetadataHandle> s_handle;
	if(s_handle == nullptr) {
		std::unique_ptr<Metadata> meta(new Metadata("network"));
		s_handle = std::unique_ptr<MetadataHandle>(new MetadataHandle(std::move(meta)));

		MetadataRegister::singleton().add(*s_handle);
	}

	return *s_handle;
}

}
