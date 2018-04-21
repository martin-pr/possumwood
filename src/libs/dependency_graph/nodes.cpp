#include "nodes.inl"

#include <cassert>

#include "graph.h"

namespace dependency_graph {

Nodes::Nodes(Network* parent) : m_parent(parent) {

}

NodeBase& Nodes::add(const MetadataHandle& type, const std::string& name, std::unique_ptr<BaseData>&& blindData, boost::optional<const dependency_graph::Datablock&> datablock, const UniqueId& id) {
	auto it = m_nodes.insert(m_parent->makeNode(name, type, id)).first;
	(*it)->m_blindData = std::move(blindData);

	if(datablock) {
		assert(&datablock->meta().metadata() == &(*it)->metadata());
		(*it)->setDatablock(*datablock);
	}

	m_parent->graph().nodeAdded(**it);
	m_parent->graph().dirtyChanged();

	return **it;
}

Nodes::iterator Nodes::erase(iterator i) {
	m_parent->graph().connections().purge(*i);

	m_parent->graph().nodeRemoved(*i);
	m_parent->graph().dirtyChanged();

	auto it = m_nodes.erase(i.base());
	return Nodes::iterator(it);
}

void Nodes::clear() {
	while(!m_nodes.empty())
		erase(Nodes::iterator(m_nodes.begin()));
}

bool Nodes::empty() const {
	return m_nodes.empty();
}

std::size_t Nodes::size() const {
	return m_nodes.size();
}

Nodes::const_iterator Nodes::begin() const {
	return Nodes::const_iterator(m_nodes.begin());
}

Nodes::const_iterator Nodes::end() const {
	return Nodes::const_iterator(m_nodes.end());
}

Nodes::const_iterator Nodes::find(const UniqueId& id) const {
	return Nodes::const_iterator(m_nodes.find(id));
}

Nodes::iterator Nodes::begin() {
	return Nodes::iterator(m_nodes.begin());
}

Nodes::iterator Nodes::end() {
	return Nodes::iterator(m_nodes.end());
}

Nodes::iterator Nodes::find(const UniqueId& id) {
	return Nodes::iterator(m_nodes.find(id));
}

NodeBase& Nodes::operator[](const dependency_graph::UniqueId& index) {
	auto it = m_nodes.find(index);
	assert(it != m_nodes.end());

	return **it;
}

const NodeBase& Nodes::operator[](const dependency_graph::UniqueId& index) const {
	auto it = m_nodes.find(index);
	assert(it != m_nodes.end());

	return **it;
}

}
