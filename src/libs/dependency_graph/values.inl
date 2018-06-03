#pragma once

#include "values.h"
#include "rtti.h"
#include "port.inl"

namespace dependency_graph {

template<typename T>
bool Values::isDirty(const OutAttr<T>& attr) const {
	return m_node->port(attr.offset()).isDirty();
}

template<typename T>
const T& Values::get(const InAttr<T>& attr) const {
	return m_node->port(attr.offset()).template get<T>();
}

template<typename T>
const T& Values::get(const OutAttr<T>& attr) const {
	return m_node->port(attr.offset()).template get<T>();
}

template<typename T>
const T& Values::get(const InAttr<void>& attr) const {
	return m_node->port(attr.offset()).template get<T>();
}

template<typename T>
void Values::set(const InAttr<T>& attr, const T& value) {
	m_node->port(attr.offset()).set(value);
}

template<typename T>
void Values::set(const OutAttr<T>& attr, const T& value) {
	m_node->port(attr.offset()).set(value);
}

template<typename T>
void Values::set(const OutAttr<void>& attr, const T& value) {
	m_node->port(attr.offset()).set(value);
}

template<typename T>
bool Values::is(const TypedAttr<void>& attr) const {
	return m_node->port(attr.offset()).type() == unmangledTypeId<T>();
}

}
