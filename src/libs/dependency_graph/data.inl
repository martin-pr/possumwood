#pragma once

#include <sstream>
#include <cassert>

#include "data.h"
#include "rtti.h"

namespace dependency_graph {

template<typename T>
Factory<T> TypedHolder<T>::m_factory;

// template<typename T>
// Data::Data(const T& value) : m_data(new TypedHolder<T>(value)) {
// }

template<typename T>
const T& Data::get() const {
	if(std::string(typeinfo().name()) == typeid(void).name() || m_data == nullptr)
		throw std::runtime_error("Attempting to use a void value!");

	const TypedHolder<T>* tmp = dynamic_cast<const TypedHolder<T>*>(m_data.get());
	if(!tmp)
		throw std::runtime_error(std::string("Invalid type requested - requested ") + typeid(T).name() + ", found " + typeinfo().name());

	return tmp->data;
}

template<typename T>
void Data::set(const T& val) {
	assert(std::string(typeid(T).name()) == std::string(typeinfo().name()));

	m_data = std::shared_ptr<const Holder>(new TypedHolder<T>(val));
}

namespace {

template<typename T>
Data makeData() {
	return Data(T());
}

template<>
Data makeData<void>() {
	return Data();
}

}

template<typename T>
Factory<T>::Factory() {
	Data::factories().insert(std::make_pair(unmangledTypeId<T>(), []() {
		return makeData<T>();
	}));
}

////////////

template<typename T>
TypedHolder<T>::TypedHolder(const T& d) : data(d) {
}

template<typename T>
TypedHolder<T>::~TypedHolder() {
	// just do something with the factory, to make sure it is instantiated
	std::stringstream ss;
	ss << &m_factory;
}

template<typename T>
const std::type_info& TypedHolder<T>::typeinfo() const {
	return typeid(T);
}

template<typename T>
bool TypedHolder<T>::isEqual(const Holder& src) const {
	const TypedHolder<T>* h2 = dynamic_cast<const TypedHolder<T>*>(&src);
	return h2 != nullptr && DataTraits<T>::isEqual(data, h2->data);
}

template<typename T>
std::string TypedHolder<T>::toString() const {
	std::stringstream ss;
	ss << data;
	return ss.str();
}

}
