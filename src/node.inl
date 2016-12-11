#pragma once

#include "node.h"

template<typename T>
void Node::Port::set(const T& value) {
	m_parent->m_data.set<T>(m_id, value);
}

template<typename T>
const T& Node::Port::get() {
	return m_parent->m_data.get<T>(m_id);
}
