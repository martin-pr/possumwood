#pragma once

#include <sstream>
#include <cassert>

#include "data.h"
#include "data_traits.h"
#include "rtti.h"

namespace dependency_graph {

template<typename T>
BaseData::Factory<T> TypedData<T>::m_factory;

template<typename T>
TypedData<T>::TypedData(const T& v) : m_value(v) {
}

template<typename T>
TypedData<T>::~TypedData() {
	// just do something with the factory, to make sure it is instantiated
	std::stringstream ss;
	ss << &m_factory;
}

template<typename T>
void TypedData<T>::assign(const BaseData& src) {
	assert(dynamic_cast<const TypedData<T>*>(&src) != NULL);

	const TypedData<T>& srcData = dynamic_cast<const TypedData<T>&>(src);
	DataTraits<T>::assignValue(m_value, srcData.m_value);
}

template<typename T>
bool TypedData<T>::isEqual(const BaseData& src) const {
	assert(dynamic_cast<const TypedData<T>*>(&src) != NULL);

	const TypedData<T>& srcData = dynamic_cast<const TypedData<T>&>(src);
	return DataTraits<T>::isEqual(m_value, srcData.m_value);
}

template<typename T>
const std::type_info& TypedData<T>::typeinfo() const {
	return typeid(T);
}

template<typename T>
std::unique_ptr<BaseData> TypedData<T>::clone() const {
	std::unique_ptr<BaseData> result(new TypedData<T>());
	result->assign(*this);

	return result;
}

template<typename T>
std::string TypedData<T>::toString() const {
	std::stringstream ss;
	ss << m_value;

	return ss.str();
}

template<typename T>
const T& TypedData<T>::get() const {
	return m_value;
}

template<typename T>
void TypedData<T>::set(const T& val) {
	m_value = val;
}

template<typename T>
BaseData::Factory<T>::Factory() {
	factories().insert(std::make_pair(unmangledTypeId<T>(), []() -> std::unique_ptr<BaseData> {
		return std::unique_ptr<BaseData>(new TypedData<T>());
	}));
}

}
