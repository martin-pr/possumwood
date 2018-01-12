#pragma once

#include "property.h"

#include "properties.h"
#include "property_item.inl"

namespace possumwood {

template <typename T>
const T& Property<T>::get(const PropertyKey& key) const {
	if(key.isDefault())
		return m_defaultValue;
	return parent().m_data[key.m_index].template get<T>(index());
}

template <typename T>
void Property<T>::set(PropertyKey& key, const T& value) {
	if(key.isDefault())
		key.m_index = parent().addSingleItem();
	assert(!key.isDefault());

	parent().m_data[key.m_index].set(index(), value);
}

template <typename T>
Property<T>::Property(Properties* parent, std::size_t index, const T& defaultValue)
    : PropertyBase(parent, index), m_defaultValue(defaultValue) {
}

template <typename T>
std::unique_ptr<PropertyBase> Property<T>::clone(Properties* parent) const {
	return std::unique_ptr<PropertyBase>(
	    new Property<T>(parent, index(), m_defaultValue));
}

template<typename T>
std::unique_ptr<PropertyItem::ValueBase> Property<T>::makeValue() const {
	return std::unique_ptr<PropertyItem::ValueBase>(new PropertyItem::Value<T>(m_defaultValue));
}
}
