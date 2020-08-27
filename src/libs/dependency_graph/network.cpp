#include "network.h"

#include "graph.h"
#include "metadata_register.h"
#include "node_base.inl"
#include "nodes.inl"

namespace dependency_graph {

Network::Network(const std::string& name, const UniqueId& id, const MetadataHandle& md, Network* parent)
    : NodeBase(name, id, md, parent), m_nodes(this), m_connections(this) {
}

Network::~Network() {
	clear();
}

bool Network::empty() const {
	return m_nodes.empty() && m_connections.empty();
}

void Network::clear() {
	// disconnect everything first
	while(!m_connections.empty()) {
		auto& conn = *m_connections.begin();

		conn.first.disconnect(conn.second);
	}

	// first, unlink all the ports
	for(unsigned i = 0; i < portCount(); ++i) {
		Port& p = port(i);
		if(p.isLinked())
			p.unlink();
	}

	// clear all subnetworks
	for(auto& n : m_nodes)
		if(n.is<Network>())
			n.as<Network>().clear();

	// clear nodes
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

void Network::setSource(const boost::filesystem::path& path) {
	m_source = path;
}

const boost::filesystem::path& Network::source() const {
	return m_source;
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

}  // namespace dependency_graph
