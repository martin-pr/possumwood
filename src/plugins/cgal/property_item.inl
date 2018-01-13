#pragma once

#include "property_item.h"

#include <cassert>

namespace possumwood {

template <typename T>
const T& PropertyItem::get(std::size_t index) const {
	assert(m_values.size() > index);
	const Value<T>& val = dynamic_cast<const Value<T>&>(*m_values[index]);

	return *val;
}

template <typename T>
void PropertyItem::set(std::size_t index, const T& value) {
	assert(m_values.size() > index);
	Value<T>& val = dynamic_cast<Value<T>&>(*m_values[index]);

	*val = value;
}

template <typename T>
void PropertyItem::addValue(const T& value) {
	std::unique_ptr<ValueBase> val(new Value<T>(value));
	m_values.push_back(std::move(val));
}

template <typename T>
PropertyItem::Value<T>::Value(const T& val) : m_value(val) {
}

template <typename T>
PropertyItem::Value<T>::~Value() {
}

template <typename T>
T& PropertyItem::Value<T>::operator*() {
	return m_value;
}

template <typename T>
const T& PropertyItem::Value<T>::operator*() const {
	return m_value;
}

template <typename T>
std::unique_ptr<PropertyItem::ValueBase> PropertyItem::Value<T>::clone() const {
	return std::unique_ptr<ValueBase>(new Value<T>(m_value));
}

template <typename T>
bool PropertyItem::Value<T>::isEqual(const ValueBase& v) const {
	const PropertyItem::Value<T>* ptr = dynamic_cast<const PropertyItem::Value<T>*>(&v);
	return ptr && m_value == ptr->m_value;
}
}
