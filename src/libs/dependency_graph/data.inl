#pragma once

#include "data.h"
#include "data_traits.h"
#include "rtti.h"

namespace dependency_graph {

template<typename T>
BaseData::Factory<T> Data<T>::m_factory;

template<typename T>
Data<T>::Data(const T& v) : value(v) {
}

template<typename T>
Data<T>::~Data() {
	// just do something with the factory, to make sure it is instantiated
	std::stringstream ss;
	ss << &m_factory;
}

template<typename T>
void Data<T>::assign(const BaseData& src) {
	assert(dynamic_cast<const Data<T>*>(&src) != NULL);

	const Data<T>& srcData = dynamic_cast<const Data<T>&>(src);
	DataTraits<T>::assignValue(value, srcData.value);
}

template<typename T>
bool Data<T>::isEqual(const BaseData& src) const {
	assert(dynamic_cast<const Data<T>*>(&src) != NULL);

	const Data<T>& srcData = dynamic_cast<const Data<T>&>(src);
	return DataTraits<T>::isEqual(value, srcData.value);
}

template<typename T>
const std::type_info& Data<T>::typeinfo() const {
	return typeid(T);
}

template<typename T>
std::unique_ptr<BaseData> Data<T>::clone() const {
	std::unique_ptr<BaseData> result(new Data<T>());
	result->assign(*this);

	return result;
}

template<typename T>
BaseData::Factory<T>::Factory() {
	factories().insert(std::make_pair(unmangledTypeId<T>(), []() -> std::unique_ptr<BaseData> {
		return std::unique_ptr<BaseData>(new Data<T>());
	}));
}

}
