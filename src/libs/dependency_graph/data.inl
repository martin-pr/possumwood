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
void Data<T>::toJson(io::json& j) const {
	j = value;
}

template<typename T>
void Data<T>::fromJson(const io::json& j) {
	DataTraits<T>::assignValue(value, j.get<T>());
}

template<typename T>
std::string Data<T>::type() const {
	return unmangledTypeId<T>();
}

template<typename T>
std::unique_ptr<BaseData> Data<T>::clone() const {
	std::unique_ptr<BaseData> result = create(type());
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
