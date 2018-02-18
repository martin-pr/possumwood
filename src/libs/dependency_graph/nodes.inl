#pragma once

#include "nodes.h"

#include "graph.h"

namespace dependency_graph {

template<typename T>
Node& Nodes::add(const Metadata& type, const std::string& name, const T& blindData, boost::optional<const dependency_graph::Datablock&> datablock) {
	m_nodes.push_back(m_parent->makeNode(name, &type));

	m_nodes.back()->m_blindData = std::unique_ptr<BaseData>(
		new Data<T>{blindData});

	if(datablock) {
		assert(&datablock->meta() == &m_nodes.back()->metadata());
		m_nodes.back()->m_data = *datablock;
	}

	m_parent->m_onAddNode(*m_nodes.back());
	m_parent->m_onDirty();

	return *m_nodes.back();
}

}
