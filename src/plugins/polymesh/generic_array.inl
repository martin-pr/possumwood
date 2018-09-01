#pragma once

#include <cassert>

#include "generic_array.h"

namespace possumwood {
namespace polymesh {

template<typename T>
Array<T>::Array(const T& defaultValue) : m_defaultValue(defaultValue) {
}

template<typename T>
Array<T>::~Array() {
}

template<typename T>
const std::type_index Array<T>::type() const {
	return typeid(T);
}

template<typename T>
std::unique_ptr<ArrayBase> Array<T>::clone() const {
	return std::unique_ptr<ArrayBase>(new Array<T>(*this));
}

template<typename T>
const T& Array<T>::operator[](std::size_t index) const {
	assert(index < m_data.size());
	return m_data[index];
}

template<typename T>
T& Array<T>::operator[](std::size_t index) {
	assert(index < m_data.size());
	return m_data[index];
}

template<typename T>
bool Array<T>::empty() const {
	return m_data.empty();
}

template<typename T>
std::size_t Array<T>::size() const {
	return m_data.size();
}

template<typename T>
void Array<T>::add(/*default*/) {
	m_data.push_back(m_defaultValue);
}

template<typename T>
void Array<T>::add(const T& value) {
	m_data.push_back(value);
}

template<typename T>
void Array<T>::clear() {
	m_data.clear();
}

template<typename T>
void Array<T>::resize(std::size_t size) {
	m_data.resize(size, m_defaultValue);
}

template<typename T>
bool Array<T>::isEqual(const ArrayBase& b) const {
	const Array<T>* arr = dynamic_cast<const Array<T>*>(&b);
	if(!arr)
		return false;

	if(m_defaultValue != arr->m_defaultValue)
		return false;

	if(m_data.size() != arr->size())
		return false;

	auto it1 = m_data.begin();
	auto it2 = arr->m_data.begin();
	while(it1 != m_data.end()) {
		if(*it1 != *it2)
			return false;

		++it1;
		++it2;
	}

	return true;
}

}
}
