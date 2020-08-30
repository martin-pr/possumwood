#pragma once

#include <cassert>
#include <sstream>

#include "data.h"
#include "rtti.h"
#include "static_initialisation.h"

namespace dependency_graph {

template <typename T>
Data::Factory<T> Data::TypedHolder<T>::m_factory;

template <typename T>
Data::Data(T value)
    : m_data(new Data::TypedHolder<typename std::remove_const<typename std::remove_reference<T>::type>::type>(
          std::move(value))) {
}

template <typename T>
const T& Data::get() const {
	if(std::string(typeinfo().name()) == typeid(void).name() || m_data == nullptr)
		throw std::runtime_error("Attempting to use a void value!");

	const TypedHolder<typename std::remove_reference<typename std::remove_const<T>::type>::type>* tmp =
	    dynamic_cast<const TypedHolder<typename std::remove_reference<typename std::remove_const<T>::type>::type>*>(
	        m_data.get());
	if(!tmp)
		throw std::runtime_error(std::string("Invalid type requested - requested ") + typeid(T).name() + ", found " +
		                         m_data->typeinfo().name());

	return tmp->data;
}

template <typename T>
void Data::set(const T& val) {
	assert(std::string(typeid(T).name()) == std::string(typeinfo().name()));

	T tmp = val;
	m_data = std::shared_ptr<const Holder>(
	    new TypedHolder<typename std::remove_reference<typename std::remove_const<T>::type>::type>(std::move(tmp)));
}

template <typename T>
void Data::set(T&& val) {
	assert(std::string(typeid(T).name()) == std::string(typeinfo().name()));

	m_data = std::shared_ptr<const Holder>(
	    new TypedHolder<typename std::remove_reference<typename std::remove_const<T>::type>::type>(std::move(val)));
}

namespace {

template <typename T>
Data makeData() {
	auto tmp = typename std::remove_reference<typename std::remove_const<T>::type>::type();
	return Data(std::move(tmp));
}

template <>
Data makeData<void>() {
	return Data();
}

}  // namespace

template <typename T>
Data::Factory<T>::Factory() {
	StaticInitialisation::registerDataFactory(unmangledTypeId<T>(), []() { return makeData<T>(); });
}

////////////

template <typename T>
Data::TypedHolder<T>::TypedHolder(T&& d) : data(std::move(d)) {
}

template <typename T>
Data::TypedHolder<T>::~TypedHolder() {
	// just do something with the factory, to make sure it is instantiated
	std::stringstream ss;
	ss << &m_factory;
}

template <typename T>
const std::type_info& Data::TypedHolder<T>::typeinfo() const {
	return typeid(T);
}

template <typename T>
bool Data::TypedHolder<T>::isEqual(const Holder& src) const {
	const TypedHolder<T>* h2 = dynamic_cast<const TypedHolder<T>*>(&src);
	return h2 != nullptr && DataTraits<T>::isEqual(data, h2->data);
}

template <typename T>
std::string Data::TypedHolder<T>::toString() const {
	std::stringstream ss;
	ss << data;
	return ss.str();
}

}  // namespace dependency_graph
