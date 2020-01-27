#include "nodes.inl"

#include <cassert>

#include "graph.h"
#include "nodes_iterator.inl"

namespace dependency_graph {

namespace {
	bool isCompatible(const dependency_graph::Metadata& m1, const dependency_graph::Metadata& m2) {
		if(m1.attributeCount() != m2.attributeCount())
			return false;

		for(std::size_t i=0; i<m1.attributeCount(); ++i) {
			const Attr& a1 = m1.attr(i);
			const Attr& a2 = m2.attr(i);

			if(a1.name() != a2.name())
				return false;

			if(a1.category() != a2.category())
				return false;

			if(a1.type() != a2.type())
				return false;
		}

		return true;
	}
}

Nodes::Nodes(Network* parent) : m_parent(parent) {

}

NodeBase& Nodes::add(const MetadataHandle& type, const std::string& name, const Data& blindData, boost::optional<const dependency_graph::Datablock&> datablock, const UniqueId& id) {
	std::unique_ptr<dependency_graph::NodeBase> node = type->createNode(name, *m_parent, id);
	assert(isCompatible(node->metadata().metadata(), type.metadata()));

	node->m_blindData = blindData;

	if(datablock) {
		assert(isCompatible(datablock->meta().metadata(), (node->metadata().metadata())));
		node->setDatablock(*datablock);
	}

	auto it = m_nodes.insert(std::move(node)).first;

	m_parent->graph().nodeAdded(**it);
	m_parent->graph().dirtyChanged();

	return **it;
}

Nodes::iterator Nodes::erase(const iterator& i) {
	i->disconnectAll();

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
