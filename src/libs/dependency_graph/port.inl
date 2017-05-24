#pragma once

#include "port.h"

#include "node.h"

namespace dependency_graph {

template<typename T>
void Port::set(const T& value) {
	// can set only inputs, not outputs
	assert(category() == Attr::kInput);
	// setting a value in the middle of the graph might do
	//   weird things, so lets assert it
	assert(!m_parent->inputIsConnected(*this));

	// set the value in the data block
	m_parent->set<T>(m_id, value);

	// explicitly setting a value makes it not dirty, but makes everything that
	//   depends on it dirty
	m_parent->markAsDirty(m_id);
	setDirty(false);
}

template<typename T>
const T& Port::get() {
	// do the computation if needed, to get rid of the dirty flag
	if(m_dirty) {
		if(category() == Attr::kInput && m_parent->inputIsConnected(*this))
			m_parent->computeInput(m_id);
		else if(category() == Attr::kOutput)
			m_parent->computeOutput(m_id);
	}

	// when the computation is done, the port should not be dirty
	assert(!m_dirty || !m_parent->inputIsConnected(*this));

	// and return the value
	return m_parent->get<T>(m_id);
}

}
