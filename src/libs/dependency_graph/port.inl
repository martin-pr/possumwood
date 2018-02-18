#pragma once

#include "port.h"

#include "node.h"
#include "graph.h"

namespace dependency_graph {

template<typename T>
void Port::set(const T& value) {
	// setting a value in the middle of the graph might do
	//   weird things, so lets assert it
	assert(category() == Attr::kOutput || !isConnected());

	// set the value in the data block
	const bool valueWasSet = m_parent->get<T>(m_id) != value;
	m_parent->set<T>(m_id, value);

	// explicitly setting a value makes it not dirty, but makes everything that
	//   depends on it dirty
	m_parent->markAsDirty(m_id);
	setDirty(false);

	// call the values callback
	if(valueWasSet)
		m_valueCallbacks();
}

template<typename T>
const T& Port::get() {
	// do the computation if needed, to get rid of the dirty flag
	if(m_dirty) {
		if(category() == Attr::kInput) {
			if(isConnected())
				m_parent->computeInput(m_id);
			else
				setDirty(false);
		}
		else if(category() == Attr::kOutput)
			m_parent->computeOutput(m_id);
	}

	// when the computation is done, the port should not be dirty
	assert(!m_dirty);

	// and return the value
	return m_parent->get<T>(m_id);
}

}
