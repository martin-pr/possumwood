#include "connections.h"

#include "graph.h"
#include "port.h"

namespace dependency_graph {

Connections::PortId Connections::getId(const Port& p) const {
	return Connections::PortId{p.node().index(), p.index()};
}

const Port& Connections::getPort(const Connections::PortId& id) const {
	return m_parent->nodes()[id.nodeIndex].port(id.portIndex);
}

Port& Connections::getPort(const Connections::PortId& id) {
	return m_parent->nodes()[id.nodeIndex].port(id.portIndex);
}

Connections::Connections(Network* parent) : m_parent(parent) {
}

void Connections::add(Port& src, Port& dest) {
	if(src.category() != Attr::kOutput)
		throw std::runtime_error("Attempting to connect an input as the origin of a connection.");

	if(dest.category() != Attr::kInput)
		throw std::runtime_error("Attempting to connect an output as the target of a connection.");

	// make a new connection
	m_connections.left.insert(std::make_pair(getId(src), getId(dest)));

	// and run the callback
	m_parent->graph().connected(src, dest);
}

void Connections::remove(Port& src, Port& dest) {
	if(src.category() != Attr::kOutput)
		throw std::runtime_error("Attempting to connect an input as the origin of a connection.");

	if(dest.category() != Attr::kInput)
		throw std::runtime_error("Attempting to connect an output as the target of a connection.");

	// try to find the connection (from right - only one connection allowed that way)
	auto it = m_connections.right.find(getId(dest));
	if((it == m_connections.right.end()) || (it->second != getId(src)))
		throw std::runtime_error("Attempting to remove a non-existing connection.");

	// run the callback
	m_parent->graph().disconnected(src, dest);

	// and remove it
	m_connections.right.erase(it);
}

bool Connections::isConnected(const NodeBase& n) const {
	for(auto& c : m_connections.left)
		if(c.first.nodeIndex == n.index() || c.second.nodeIndex == n.index())
			return true;
	return false;
}

boost::optional<const Port&> Connections::connectedFrom(const Port& p) const {
	auto it = m_connections.right.lower_bound(getId(p));

	if((it == m_connections.right.end()) || (it->first != getId(p)))
		return boost::optional<const Port&>();
	else {
		assert(m_connections.right.count(getId(p)) == 1);
		return getPort(it->second);
	}
}

std::vector<std::reference_wrapper<const Port>> Connections::connectedTo(const Port& p) const {
	auto it1 = m_connections.left.lower_bound(getId(p));
	auto it2 = m_connections.left.upper_bound(getId(p));

	std::vector<std::reference_wrapper<const Port>> result;
	for(auto it = it1; it != it2; ++it)
		result.push_back(std::cref(getPort(it->second)));
	return result;
}

boost::optional<Port&> Connections::connectedFrom(Port& p) {
	if(p.category() != Attr::kInput)
		throw std::runtime_error("Connected From request can be only run on input ports.");

	auto it = m_connections.right.lower_bound(getId(p));

	if((it == m_connections.right.end()) || (it->first != getId(p)))
		return boost::optional<Port&>();
	else {
		assert(m_connections.right.count(getId(p)) == 1);
		return getPort(it->second);
	}
}

std::vector<std::reference_wrapper<Port>> Connections::connectedTo(Port& p) {
	if(p.category() != Attr::kOutput)
		throw std::runtime_error("Connected To request can be only run on output ports.");

	auto it1 = m_connections.left.lower_bound(getId(p));
	auto it2 = m_connections.left.upper_bound(getId(p));

	std::vector<std::reference_wrapper<Port>> result;
	for(auto it = it1; it != it2; ++it)
		result.push_back(std::ref(getPort(it->second)));
	return result;
}

size_t Connections::size() const {
	return m_connections.size();
}

bool Connections::empty() const {
	return m_connections.empty();
}

/// dereferencing function to convert internal connection representation to a pair of port references
const std::pair<const Port&, const Port&> Connections::convert(
    const Connections::connections_container::left_value_type& val) const {
	const Port& left = getPort(val.first);
	const Port& right = getPort(val.second);

	return std::pair<const Port&, const Port&>(left, right);
}

/// dereferencing function to convert internal connection representation to a pair of port references
const std::pair<Port&, Port&> Connections::convertConst(
    const Connections::connections_container::left_value_type& val) {
	Port& left = getPort(val.first);
	Port& right = getPort(val.second);

	return std::pair<Port&, Port&>(left, right);
}

Connections::const_iterator Connections::begin() const {
	auto fn = [this](const Connections::connections_container::left_value_type& val) { return convert(val); };
	return const_iterator(m_connections.left.begin(), fn);
}

Connections::const_iterator Connections::end() const {
	auto fn = [this](const Connections::connections_container::left_value_type& val) { return convert(val); };
	return const_iterator(m_connections.left.end(), fn);
}

Connections::iterator Connections::begin() {
	auto fn = [this](const Connections::connections_container::left_value_type& val) { return convertConst(val); };
	return iterator(m_connections.left.begin(), fn);
}

Connections::iterator Connections::end() {
	auto fn = [this](const Connections::connections_container::left_value_type& val) { return convertConst(val); };
	return iterator(m_connections.left.end(), fn);
}

}  // namespace dependency_graph
