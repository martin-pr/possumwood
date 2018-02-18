#include "connections.h"

#include "port.h"
#include "graph.h"

namespace dependency_graph {

Connections::Connections() {
}

void Connections::add(Port& src, Port& dest) {
	if(src.category() != Attr::kOutput)
		throw std::runtime_error("Attempting to connect an input as the origin of a connection.");

	if(dest.category() != Attr::kInput)
		throw std::runtime_error("Attempting to connect an output as the target of a connection.");

	// make a new connection
	m_connections.left.insert(std::make_pair(&src, &dest));

	// and run the callback
	m_onConnect(src, dest);
}

void Connections::remove(Port& src, Port& dest) {
	if(src.category() != Attr::kOutput)
		throw std::runtime_error("Attempting to connect an input as the origin of a connection.");

	if(dest.category() != Attr::kInput)
		throw std::runtime_error("Attempting to connect an output as the target of a connection.");

	// try to find the connection (from right - only one connection allowed that way)
	auto it = m_connections.right.find(&dest);
	if((it == m_connections.right.end()) || (it->second != &src))
		throw std::runtime_error("Attempting to remove a non-existing connection.");

	// run the callback
	m_onDisconnect(src, dest);

	// and remove it
	m_connections.right.erase(it);
}

void Connections::purge(const Node& n) {
	// remove all connections related to a node
	auto it = m_connections.left.begin();
	while(it != m_connections.left.end())
		if((&(it->first->node()) == &n) || (&(it->second->node()) == &n)) {
			m_onDisconnect(*it->first, *it->second);

			// remove the iterator
			it = m_connections.left.erase(it);
		}
		else
			++it;
}

boost::optional<const Port&> Connections::connectedFrom(const Port& p) const {
	if(p.category() != Attr::kInput)
		throw std::runtime_error("Connected From request can be only run on input ports.");

	// ugly const casts, to keep const-correctness, ironically
	auto it = m_connections.right.lower_bound(const_cast<Port*>(&p));

	if((it == m_connections.right.end()) || (it->first != &p))
		return boost::optional<const Port&>();
	else {
		assert(m_connections.right.count(const_cast<Port*>(&p)) == 1);
		return *it->second;
	}
}

std::vector<std::reference_wrapper<const Port>> Connections::connectedTo(const Port& p) const {
	if(p.category() != Attr::kOutput)
		throw std::runtime_error("Connected To request can be only run on output ports.");

	// ugly const casts, to keep const-correctness, ironically
	auto it1 = m_connections.left.lower_bound(const_cast<Port*>(&p));
	auto it2 = m_connections.left.upper_bound(const_cast<Port*>(&p));

	std::vector<std::reference_wrapper<const Port>> result;
	for(auto it = it1; it != it2; ++it)
		result.push_back(std::cref(*it->second));
	return result;
}

boost::optional<Port&> Connections::connectedFrom(Port& p) {
	if(p.category() != Attr::kInput)
		throw std::runtime_error("Connected From request can be only run on input ports.");

	// ugly const casts, to keep const-correctness, ironically
	auto it = m_connections.right.lower_bound(&p);

	if((it == m_connections.right.end()) || (it->first != &p))
		return boost::optional<Port&>();
	else {
		assert(m_connections.right.count(&p) == 1);
		return *it->second;
	}
}

std::vector<std::reference_wrapper<Port>> Connections::connectedTo(Port& p) {
	if(p.category() != Attr::kOutput)
		throw std::runtime_error("Connected To request can be only run on output ports.");

	auto it1 = m_connections.left.lower_bound(&p);
	auto it2 = m_connections.left.upper_bound(&p);

	std::vector<std::reference_wrapper<Port>> result;
	for(auto it = it1; it != it2; ++it)
		result.push_back(std::ref(*it->second));
	return result;
}

size_t Connections::size() const {
	return m_connections.size();
}

bool Connections::empty() const {
	return m_connections.empty();
}

namespace {
	/// dereferencing function to convert internal connection representation to a pair of port references
	const std::pair<const Port&, const Port&> convert(const Connections::connections_container::left_value_type& val)	{
		const Port& left = *val.first;
		const Port& right = *val.second;

		return std::pair<const Port&, const Port&>(left, right);
	}

	/// dereferencing function to convert internal connection representation to a pair of port references
	const std::pair<Port&, Port&> convertConst(const Connections::connections_container::left_value_type& val)	{
		Port& left = *val.first;
		Port& right = *val.second;

		return std::pair<Port&, Port&>(left, right);
	}
}

Connections::const_iterator Connections::begin() const {
	return const_iterator(m_connections.left.begin(), convert);
}

Connections::const_iterator Connections::end() const {
	return const_iterator(m_connections.left.end(), convert);
}

Connections::iterator Connections::begin() {
	return iterator(m_connections.left.begin(), convertConst);
}

Connections::iterator Connections::end() {
	return iterator(m_connections.left.end(), convertConst);
}

boost::signals2::connection Connections::onConnect(std::function<void(Port&, Port&)> callback) {
	return m_onConnect.connect(callback);
}

boost::signals2::connection Connections::onDisconnect(std::function<void(Port&, Port&)> callback) {
	return m_onDisconnect.connect(callback);
}

}
