#include "factory.h"

#include <cassert>

#include <dependency_graph/rtti.h>

namespace possumwood { namespace properties {

template<typename T>
std::string factory_typed<T>::type() const {
	return m_type;
}

template<typename T>
std::unique_ptr<property_base> factory_typed<T>::create() {
	return std::unique_ptr<property_base>(new T());
}

template<typename T>
factory_typed<T>::factory_typed() {
	m_type = dependency_graph::unmangledName(typeid(typename T::result_type).name());

	factories::singleton().add(this);
}

template<typename T>
factory_typed<T>::~factory_typed() {
	factories::singleton().remove(this);
}

} }
