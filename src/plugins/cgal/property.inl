#pragma once

#include "properties.h"
#include "property.h"

namespace possumwood {

template <typename T>
const T& Property<T>::get(const PropertyKey& key) const {
	if(key.isDefault())
		return m_defaultValue;
	assert(key.m_index < (int)m_data.size());
	return m_data[key.m_index];
}

template <typename T>
void Property<T>::set(PropertyKey& key, const T& value) {
	if(key.isDefault())
		key.m_index = m_data.size();

	m_data.push_back(value);
}

template <typename T>
void Property<T>::set(const PropertyKey& key, const T& value) {
	assert(!key.isDefault());
	assert(key.m_index < m_data.size());

	m_data[key.m_index] = value;
}

template <typename T>
Property<T>::Property(const std::string& name, const T& defaultValue)
    : PropertyBase(name, typeid(T)), m_defaultValue(defaultValue) {
}

template <typename T>
std::unique_ptr<PropertyBase> Property<T>::clone() const {
	std::unique_ptr<Property<T>> result(new Property<T>(name(), m_defaultValue));
	result->m_data = m_data;
	return std::unique_ptr<PropertyBase>(result.release());
}

template <typename T>
bool Property<T>::isEqual(const PropertyBase& p) const {
	if(name() != p.name() || type() != p.type())
		return false;

	const Property<T>& prop = dynamic_cast<const Property<T>&>(p);
	if(m_defaultValue != prop.m_defaultValue || m_data.size() != prop.m_data.size())
		return false;

	auto it1 = m_data.begin();
	auto it2 = prop.m_data.begin();
	for(; it1 != m_data.end(); ++it1, ++it2) {
		if(*it1 != *it2)
			return false;
	}

	return true;
}

template <typename T>
typename Property<T>::iterator Property<T>::begin() {
	return m_data.begin();
}

template <typename T>
typename Property<T>::iterator Property<T>::end() {
	return m_data.end();
}

}  // namespace possumwood
