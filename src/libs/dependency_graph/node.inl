#pragma once

#include "node.h"
#include "graph.h"

#include "datablock.inl"

namespace dependency_graph {

template<typename T>
const T& Node::get(size_t index) const {
	// assert(!port(index).isDirty() || (port(index).category() == Attr::kInput && !inputIsConnected(port(index))));
	return m_data.get<T>(index);
}

template<typename T>
void Node::set(size_t index, const T& value) {
	assert(port(index).category() == Attr::kOutput || !inputIsConnected(port(index)));
	m_data.set<T>(index, value);
}

template<typename T>
void Node::setBlindData(const T& value) {
	// create blind data if they're not present
	if(m_blindData.get() == NULL)
		m_blindData = std::unique_ptr<BaseData>(new Data<T>());

	// retype
	Data<T>& val = dynamic_cast<Data<T>&>(*m_blindData);

	// set the value
	if(val.value != value) {
		val.value = value;

		// and call the callback
		m_parent->m_onBlindDataChanged(*this);
	}
}

/// blind per-node data, to be used by the client application
///   to store visual information (e.g., node position, colour...)
template<typename T>
const T& Node::blindData() const {
	// retype and return
	assert(m_blindData != NULL);
	assert(m_blindData->type() == unmangledTypeId<T>());
	const Data<T>& val = dynamic_cast<const Data<T>&>(*m_blindData);
	return val.value;
}

}
