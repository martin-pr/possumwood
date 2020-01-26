#pragma once

#include <sstream>
#include <cassert>

#include "data.h"
#include "rtti.h"

namespace dependency_graph {

template<typename T>
Data::Factory<T> Data::TypedHolder<T>::m_factory;

// template<typename T>
// TypedData<T>::TypedData(const T& v) {
// 	m_data = std::shared_ptr<Holder>(new TypedHolder<T>(v));
// }

// template<typename T>
// TypedData<T>::~TypedData() {
// }

// template<typename T>
// void TypedData<T>::assign(const Data& src) {
// 	assert(dynamic_cast<const TypedData<T>*>(&src) != NULL);

// 	m_data = src.m_data;
// }

// template<typename T>
// bool TypedData<T>::isEqual(const Data& src) const {
// 	assert(dynamic_cast<const TypedData<T>*>(&src) != NULL);

// 	const Data::TypedHolder<T>& h1 = dynamic_cast<const Data::TypedHolder<T>&>(*m_data);
// 	const Data::TypedHolder<T>& h2 = dynamic_cast<const Data::TypedHolder<T>&>(*src.m_data);

// 	return DataTraits<T>::isEqual(h1.data, h2.data);
// }

// template<typename T>
// const std::type_info& TypedData<T>::typeinfo() const {
// 	return typeid(T);
// }

// template<typename T>
// std::unique_ptr<Data> TypedData<T>::clone() const {
// 	std::unique_ptr<Data> result(new TypedData<T>());
// 	result->assign(*this);

// 	return result;
// }

// template<typename T>
// std::string TypedData<T>::toString() const {
// 	return m_data->toString();
// }

// template<typename T>
// const T& TypedData<T>::get() const {
// 	const Data::TypedHolder<T>& h = dynamic_cast<const Data::TypedHolder<T>&>(m_data);
// 	return h;
// }

// template<typename T>
// void TypedData<T>::set(const T& val) {
// 	assert(std::string(typeinfo().name()) == std::string(typeid(T).name()));

// 	m_data = std::shared_ptr<const Holder>(new TypedHolder<T>(val));
// }

template<typename T>
Data::Factory<T>::Factory() {
	factories().insert(std::make_pair(unmangledTypeId<T>(), []() -> std::unique_ptr<Data> {
		Data d;
		d.m_data = std::shared_ptr<const Data::Holder>(new Data::TypedHolder<T>(T()));

		return std::unique_ptr<Data>(new Data(d));
	}));
}

// template<typename T>
// Data::TypedHolder<T>::~TypedHolder() {
// 	// just do something with the factory, to make sure it is instantiated
// 	std::stringstream ss;
// 	ss << &m_factory;
// }

}
