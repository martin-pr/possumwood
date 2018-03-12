#pragma once

#include "node.h"
#include "graph.h"

#include "datablock.inl"
#include "node_base.inl"

namespace dependency_graph {

template<typename T>
const T& Node::get(size_t index) const {
	// assert(!port(index).isDirty() || (port(index).category() == Attr::kInput && !inputIsConnected(port(index))));
	return m_data.get<T>(index);
}

template<typename T>
void Node::set(size_t index, const T& value) {
	assert(port(index).category() == Attr::kOutput || !port(index).isConnected());
	m_data.set<T>(index, value);
}

}
