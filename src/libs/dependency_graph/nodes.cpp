#include "nodes.inl"

#include "graph.h"

namespace dependency_graph {

Nodes::Nodes(Graph* parent) : m_parent(parent) {

}

Node& Nodes::add(const Metadata& type, const std::string& name, std::unique_ptr<BaseData>&& blindData, boost::optional<const dependency_graph::Datablock&> datablock) {
	m_nodes.push_back(m_parent->makeNode(name, &type));
	m_nodes.back()->m_blindData = std::move(blindData);

	if(datablock) {
		assert(&datablock->meta() == &m_nodes.back()->metadata());
		m_nodes.back()->m_data = *datablock;
	}

	m_parent->nodeAdded(*m_nodes.back());
	m_parent->dirtyChanged();

	return *m_nodes.back();
}

Nodes::iterator Nodes::erase(iterator i) {
	m_parent->connections().purge(*i);

	m_parent->nodeRemoved(*i);
	m_parent->dirtyChanged();

	auto it = m_nodes.erase(i.base());
	return boost::make_indirect_iterator(it);
}

void Nodes::clear() {
	while(!m_nodes.empty())
		erase(boost::make_indirect_iterator(m_nodes.end() - 1));
}

bool Nodes::empty() const {
	return m_nodes.empty();
}

std::size_t Nodes::size() const {
	return m_nodes.size();
}

Nodes::const_iterator Nodes::begin() const {
	return boost::make_indirect_iterator(m_nodes.begin());
}

Nodes::const_iterator Nodes::end() const {
	return boost::make_indirect_iterator(m_nodes.end());
}

Nodes::iterator Nodes::begin() {
	return boost::make_indirect_iterator(m_nodes.begin());
}

Nodes::iterator Nodes::end() {
	return boost::make_indirect_iterator(m_nodes.end());
}

Node& Nodes::operator[](std::size_t index) {
	assert(index < m_nodes.size());
	return *m_nodes[index];
}

const Node& Nodes::operator[](std::size_t index) const {
	assert(index < m_nodes.size());
	return *m_nodes[index];
}

size_t Nodes::findNodeIndex(const NodeBase& n) const {
	auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&](const std::unique_ptr<Node>& ptr) {
		return ptr.get() == &n;
	});

	assert(it != m_nodes.end() && "node not found");
	return it - m_nodes.begin();
}

}
