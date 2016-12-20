#pragma once

#include "node.h"

template<typename T>
void Node::Port::set(const T& value) {
	// setting a value in the middle of the graph might do
	//   weird things, so lets assert it
	assert(m_parent->inputIsNotConnected(*this));

	// set the value in the data block
	m_parent->set<T>(m_id, value);

	// explicitly setting a value makes it not dirty
	m_dirty = false;
}

template<typename T>
const T& Node::Port::get() {
	const T& val = m_parent->get<T>(m_id, m_dirty);
	m_dirty = false;
	return val;
}
}
