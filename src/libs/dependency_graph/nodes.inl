#pragma once

#include "nodes.h"

#include "graph.h"

#include "node_base.inl"

namespace dependency_graph {

template<typename T>
NodeBase& Nodes::add(const MetadataHandle& type, const std::string& name, const T& blindData, boost::optional<const dependency_graph::Datablock&> datablock) {
	std::unique_ptr<NodeBase> node = m_parent->makeNode(name, type);
	node->setBlindData(blindData);

	assert(m_nodes.find(node->index()) == m_nodes.end());
	auto it = m_nodes.insert(std::move(node)).first;

	if(datablock) {
		assert(datablock->meta() == (*it)->metadata());
		(*it)->setDatablock(*datablock);
	}

	m_parent->graph().nodeAdded(**it);
	m_parent->graph().dirtyChanged();

	return **it;
}

}
