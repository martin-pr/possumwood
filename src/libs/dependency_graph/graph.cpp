#include "graph.h"

#include <algorithm>

namespace dependency_graph {

Graph::Graph() : m_nodes(this), m_connections(this) {

}

bool Graph::empty() const {
	return m_nodes.empty();
}

void Graph::clear() {
	// clear only nodes - connections will clear themselves with the nodes
	m_nodes.clear();
}

Graph::Nodes& Graph::nodes() {
	return m_nodes;
}

const Graph::Nodes& Graph::nodes() const {
	return m_nodes;
}

Graph::Connections& Graph::connections() {
	return m_connections;
}

const Graph::Connections& Graph::connections() const {
	return m_connections;
}

std::unique_ptr<Node> Graph::makeNode(const std::string& name, const Metadata* md) {
	return std::unique_ptr<Node>(new Node(name, md, this));
}

boost::signals2::connection Graph::onAddNode(std::function<void(Node&)> callback) {
	return m_onAddNode.connect(callback);
}

boost::signals2::connection Graph::onRemoveNode(std::function<void(Node&)> callback) {
	return m_onRemoveNode.connect(callback);
}


boost::signals2::connection Graph::onConnect(std::function<void(Port&, Port&)> callback) {
	return m_onConnect.connect(callback);
}

boost::signals2::connection Graph::onDisconnect(std::function<void(Port&, Port&)> callback) {
	return m_onDisconnect.connect(callback);
}

boost::signals2::connection Graph::onBlindDataChanged(std::function<void(Node&)> callback) {
	return m_onBlindDataChanged.connect(callback);
}

boost::signals2::connection Graph::onNameChanged(std::function<void(Node&)> callback) {
	return m_onNameChanged.connect(callback);
}

boost::signals2::connection Graph::onDirty(std::function<void()> callback) {
	return m_onDirty.connect(callback);
}

boost::signals2::connection Graph::onStateChanged(std::function<void(const Node&)> callback) {
	return m_onStateChanged.connect(callback);
}

boost::signals2::connection Graph::onLog(std::function<void(State::MessageType, const std::string&)> callback) {
	return m_onLog.connect(callback);
}

//////////////

Graph::Nodes::Nodes(Graph* parent) : m_parent(parent) {

}

Node& Graph::Nodes::add(const Metadata& type, const std::string& name, std::unique_ptr<BaseData>&& blindData, boost::optional<const dependency_graph::Datablock&> datablock) {
	m_nodes.push_back(m_parent->makeNode(name, &type));
	m_nodes.back()->m_blindData = std::move(blindData);

	if(datablock) {
		assert(&datablock->meta() == &m_nodes.back()->metadata());
		m_nodes.back()->m_data = *datablock;
	}

	m_parent->m_onAddNode(*m_nodes.back());
	m_parent->m_onDirty();

	return *m_nodes.back();
}

Graph::Nodes::iterator Graph::Nodes::erase(iterator i) {
	m_parent->m_connections.purge(*i);

	m_parent->m_onRemoveNode(*i);
	m_parent->m_onDirty();

	auto it = m_nodes.erase(i.base());
	return boost::make_indirect_iterator(it);
}

void Graph::Nodes::clear() {
	while(!m_nodes.empty())
		erase(boost::make_indirect_iterator(m_nodes.end() - 1));
}

bool Graph::Nodes::empty() const {
	return m_nodes.empty();
}

std::size_t Graph::Nodes::size() const {
	return m_nodes.size();
}

Graph::Nodes::const_iterator Graph::Nodes::begin() const {
	return boost::make_indirect_iterator(m_nodes.begin());
}

Graph::Nodes::const_iterator Graph::Nodes::end() const {
	return boost::make_indirect_iterator(m_nodes.end());
}

Graph::Nodes::iterator Graph::Nodes::begin() {
	return boost::make_indirect_iterator(m_nodes.begin());
}

Graph::Nodes::iterator Graph::Nodes::end() {
	return boost::make_indirect_iterator(m_nodes.end());
}

Node& Graph::Nodes::operator[](std::size_t index) {
	assert(index < m_nodes.size());
	return *m_nodes[index];
}

const Node& Graph::Nodes::operator[](std::size_t index) const {
	assert(index < m_nodes.size());
	return *m_nodes[index];
}

size_t Graph::Nodes::findNodeIndex(const Node& n) const {
	auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&](const std::unique_ptr<Node>& ptr) {
		return ptr.get() == &n;
	});

	assert(it != m_nodes.end() && "node not found");
	return it - m_nodes.begin();
}

//////////////

Graph::Connections::Connections(Graph* parent) : m_parent(parent) {
}

void Graph::Connections::add(Port& src, Port& dest) {
	if(src.category() != Attr::kOutput)
		throw std::runtime_error("Attempting to connect an input as the origin of a connection.");

	if(dest.category() != Attr::kInput)
		throw std::runtime_error("Attempting to connect an output as the target of a connection.");

	// make a new connection
	m_connections.left.insert(std::make_pair(&src, &dest));

	// and run the callback
	m_parent->m_onConnect(src, dest);
}

void Graph::Connections::remove(Port& src, Port& dest) {
	if(src.category() != Attr::kOutput)
		throw std::runtime_error("Attempting to connect an input as the origin of a connection.");

	if(dest.category() != Attr::kInput)
		throw std::runtime_error("Attempting to connect an output as the target of a connection.");

	// try to find the connection (from right - only one connection allowed that way)
	auto it = m_connections.right.find(&dest);
	if((it == m_connections.right.end()) || (it->second != &src))
		throw std::runtime_error("Attempting to remove a non-existing connection.");

	// run the callback
	m_parent->m_onDisconnect(src, dest);

	// and remove it
	m_connections.right.erase(it);
}

void Graph::Connections::purge(const Node& n) {
	// remove all connections related to a node
	auto it = m_connections.left.begin();
	while(it != m_connections.left.end())
		if((&(it->first->node()) == &n) || (&(it->second->node()) == &n)) {
			m_parent->m_onDisconnect(*it->first, *it->second);

			// remove the iterator
			it = m_connections.left.erase(it);
		}
		else
			++it;
}

boost::optional<const Port&> Graph::Connections::connectedFrom(const Port& p) const {
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

std::vector<std::reference_wrapper<const Port>> Graph::Connections::connectedTo(const Port& p) const {
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

boost::optional<Port&> Graph::Connections::connectedFrom(Port& p) {
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

std::vector<std::reference_wrapper<Port>> Graph::Connections::connectedTo(Port& p) {
	if(p.category() != Attr::kOutput)
		throw std::runtime_error("Connected To request can be only run on output ports.");

	auto it1 = m_connections.left.lower_bound(&p);
	auto it2 = m_connections.left.upper_bound(&p);

	std::vector<std::reference_wrapper<Port>> result;
	for(auto it = it1; it != it2; ++it)
		result.push_back(std::ref(*it->second));
	return result;
}

size_t Graph::Connections::size() const {
	return m_connections.size();
}

bool Graph::Connections::empty() const {
	return m_connections.empty();
}

namespace {
	/// dereferencing function to convert internal connection representation to a pair of port references
	const std::pair<const Port&, const Port&> convert(const Graph::Connections::connections_container::left_value_type& val)	{
		const Port& left = *val.first;
		const Port& right = *val.second;

		return std::pair<const Port&, const Port&>(left, right);
	}

	/// dereferencing function to convert internal connection representation to a pair of port references
	const std::pair<Port&, Port&> convertConst(const Graph::Connections::connections_container::left_value_type& val)	{
		Port& left = *val.first;
		Port& right = *val.second;

		return std::pair<Port&, Port&>(left, right);
	}
}

Graph::Connections::const_iterator Graph::Connections::begin() const {
	return const_iterator(m_connections.left.begin(), convert);
}

Graph::Connections::const_iterator Graph::Connections::end() const {
	return const_iterator(m_connections.left.end(), convert);
}

Graph::Connections::iterator Graph::Connections::begin() {
	return iterator(m_connections.left.begin(), convertConst);
}

Graph::Connections::iterator Graph::Connections::end() {
	return iterator(m_connections.left.end(), convertConst);
}

}
