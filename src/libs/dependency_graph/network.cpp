#include "network.h"

#include "graph.h"
#include "metadata_register.h"

namespace dependency_graph {

namespace {

const MetadataHandle& networkMetadata() {
	static std::unique_ptr<MetadataHandle> s_handle;
	if(s_handle == nullptr) {
		std::unique_ptr<Metadata> meta(new Metadata("network"));
		s_handle = std::unique_ptr<MetadataHandle>(new MetadataHandle(std::move(meta)));

		MetadataRegister::singleton().add(*s_handle);
	}

	return *s_handle;
}

}

Network::Network(Network* parent) : NodeBase("network", networkMetadata(), parent), m_nodes(this), m_connections(this) {
}

Network::~Network() {
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

Port& Network::port(size_t index) {
	assert(false);
	throw std::runtime_error("Network has no ports, for now");
}

const Port& Network::port(size_t index) const {
	assert(false);
	throw std::runtime_error("Network has no ports, for now");
}

const size_t Network::portCount() const {
	return 0;
}

void Network::computeInput(size_t index) {
	assert(false);
	throw std::runtime_error("Network has no ports, for now");
}

void Network::computeOutput(size_t index) {
	assert(false);
	throw std::runtime_error("Network has no ports, for now");
}

const State& Network::state() const {
	assert(false);
	throw std::runtime_error("Network has no ports, for now");
}

std::unique_ptr<NodeBase> Network::makeNode(const std::string& name, const MetadataHandle& md) {
	if(md != networkMetadata())
		return std::unique_ptr<NodeBase>(new Node(name, md, this));
	else
		return std::unique_ptr<NodeBase>(new Network(this));
}

}
