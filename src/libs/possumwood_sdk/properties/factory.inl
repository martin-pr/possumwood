#include <dependency_graph/rtti.h>

#include <cassert>

#include "factory.h"

namespace possumwood {
namespace properties {

template <typename T>
std::type_index factory_typed<T>::type() const {
	return m_type;
}

template <typename T>
std::type_index factory_typed<T>::uiType() const {
	return typeid(T);
}

template <typename T>
std::unique_ptr<property_base> factory_typed<T>::create() {
	return std::unique_ptr<property_base>(new T());
}

template <typename T>
factory_typed<T>::factory_typed() : m_type(typeid(typename T::result_type)) {
	factories::singleton().add(this);
}

template <typename T>
factory_typed<T>::~factory_typed() {
	factories::singleton().remove(this);
}

}  // namespace properties
}  // namespace possumwood
