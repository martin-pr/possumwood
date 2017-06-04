#pragma once

#include "data.h"

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
	value = srcData.value;
}

template<typename T>
bool Data<T>::isEqual(const BaseData& src) const {
	assert(dynamic_cast<const Data<T>*>(&src) != NULL);

	const Data<T>& srcData = dynamic_cast<const Data<T>&>(src);
	return value == srcData.value;
}

template<typename T>
void Data<T>::toJson(io::json& j) const {
	j = value;
}

template<typename T>
void Data<T>::fromJson(const io::json& j) {
	value = j.get<T>();
}

template<typename T>
std::string Data<T>::type() const {
	return typeid(T).name();
}

template<typename T>
BaseData::Factory<T>::Factory() {
	factories().insert(std::make_pair(typeid(T).name(), []() -> std::unique_ptr<BaseData> {
		return std::unique_ptr<BaseData>(new Data<T>());
	}));
}

}
