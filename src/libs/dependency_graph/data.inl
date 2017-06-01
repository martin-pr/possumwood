#pragma once

#include "data.h"

namespace dependency_graph {

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

}
