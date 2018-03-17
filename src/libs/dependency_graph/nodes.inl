#pragma once

#include "nodes.h"

#include "graph.h"

#include "node_base.inl"

namespace dependency_graph {

template<typename T>
NodeBase& Nodes::add(const Metadata& type, const std::string& name, const T& blindData, boost::optional<const dependency_graph::Datablock&> datablock) {
	m_nodes.push_back(m_parent->makeNode(name, &type));

	m_nodes.back()->m_blindData = std::unique_ptr<BaseData>(
		new Data<T>{blindData});

	if(datablock) {
		assert(&datablock->meta() == &m_nodes.back()->metadata());
		m_nodes.back()->setDatablock(*datablock);
	}

	m_parent->graph().nodeAdded(*m_nodes.back());
	m_parent->graph().dirtyChanged();

	return *m_nodes.back();
}

}
