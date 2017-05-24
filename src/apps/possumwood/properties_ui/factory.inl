#include "factory.h"

#include <cassert>

namespace properties {

template<typename T>
std::string factory_typed<T>::type() const {
	return typeid(typename T::result_type).name();
}

template<typename T>
std::unique_ptr<property_base> factory_typed<T>::create() {
	return std::unique_ptr<property_base>(new T());
}

template<typename T>
factory_typed<T>::factory_typed() {
	factories::singleton().add(this);
}

template<typename T>
factory_typed<T>::~factory_typed() {
	factories::singleton().remove(this);
}

}
