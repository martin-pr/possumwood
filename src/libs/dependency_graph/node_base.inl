#pragma once

#include "node_base.h"
#include "network.h"
#include "rtti.h"
#include "graph.h"

namespace dependency_graph {

template<typename T>
void NodeBase::setBlindData(const T& value) {
	// create blind data if they're not present
	bool newData = false;
	if(m_blindData.get() == NULL) {
		m_blindData = std::unique_ptr<BaseData>(new Data<T>());
		newData = true;
	}

	// retype
	Data<T>& val = dynamic_cast<Data<T>&>(*m_blindData);

	// set the value
	if(val.value != value) {
		val.value = value;

		// and call the callback
		if(!newData)
			network().graph().blindDataChanged(*this);
	}
}

/// blind per-node data, to be used by the client application
///   to store visual information (e.g., node position, colour...)
template<typename T>
const T& NodeBase::blindData() const {
	// retype and return
	assert(m_blindData != NULL);
	assert(m_blindData->type() == unmangledTypeId<T>());
	const Data<T>& val = dynamic_cast<const Data<T>&>(*m_blindData);
	return val.value;
}

template<typename T>
const T& NodeBase::get(size_t index) const {
	// assert(!port(index).isDirty() || (port(index).category() == Attr::kInput && !inputIsConnected(port(index))));
	return datablock().get<T>(index);
}

template<typename T>
void NodeBase::set(size_t index, const T& value) {
	assert(port(index).category() == Attr::kOutput || !port(index).isConnected());
	datablock().set<T>(index, value);
}

}
