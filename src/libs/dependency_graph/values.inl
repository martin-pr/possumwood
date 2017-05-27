#pragma once

#include "values.h"

namespace dependency_graph {

Values::Values(Node& n) : m_node(&n) {
}

template<typename T>
const T& Values::get(const InAttr<T>& attr) const {
	return 	m_node->port(attr.offset()).get<T>();
}

template<typename T>
const T& Values::get(const OutAttr<T>& attr) const {
	return 	m_node->port(attr.offset()).get<T>();
}

template<typename T>
void Values::set(const OutAttr<T>& attr, const T& value) {
	m_node->port(attr.offset()).set(value);
}

}
