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
	return Nodes::iterator(it, m_nodes.end(), false);
}

void Nodes::clear() {
	while(!m_nodes.empty())
		erase(Nodes::iterator(m_nodes.begin(), m_nodes.end(), false));
}

bool Nodes::empty() const {
	return m_nodes.empty();
}

std::size_t Nodes::size() const {
	return m_nodes.size();
}

Nodes::const_iterator Nodes::begin(const SearchType& st) const {
	return const_iterator(m_nodes.begin(), m_nodes.end(), st == kRecursive);
}

Nodes::const_iterator Nodes::end() const {
	return const_iterator(m_nodes.end(), m_nodes.end(), false);
}

Nodes::const_iterator Nodes::find(const UniqueId& id, const SearchType& st) const {
	if(st == kThisNetwork)
		return const_iterator(m_nodes.find(id), m_nodes.end(), false);

	// linear search, for now
	else {
		auto it = begin(kRecursive);
		while(it != end() && it->index() != id)
			++it;
		return it;
	}
}

Nodes::iterator Nodes::begin(const SearchType& st) {
	return iterator(m_nodes.begin(), m_nodes.end(), st == kRecursive);
}

Nodes::iterator Nodes::end() {
	return Nodes::iterator(m_nodes.end(), m_nodes.end(), false);
}

Nodes::iterator Nodes::find(const UniqueId& id, const SearchType& st) {
	if(st == kThisNetwork)
		return Nodes::iterator(m_nodes.find(id), m_nodes.end(), false);

	// linear search, for now
	else {
		auto it = begin(kRecursive);
		while(it != end() && it->index() != id)
			++it;
		return it;
	}
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
