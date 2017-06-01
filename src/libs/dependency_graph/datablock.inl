#pragma once

#include "datablock.h"

#include <cassert>

#include "metadata.h"

namespace dependency_graph {

template<typename T>
const T& Datablock::get(size_t index) const {
	assert(index < m_data.size());
	Data<T>& data = dynamic_cast<Data<T>&>(*m_data[index]);
	return data.value;
}

template<typename T>
void Datablock::set(size_t index, const T& value) {
	assert(index < m_data.size());
	Data<T>& data = dynamic_cast<Data<T>&>(*m_data[index]);
	data.value = value;
}

template<typename T>
void Datablock::Data<T>::assign(const BaseData& src) {
	assert(dynamic_cast<const Data<T>*>(&src) != NULL);

	const Data<T>& srcData = dynamic_cast<const Data<T>&>(src);
	value = srcData.value;
}

template<typename T>
bool Datablock::Data<T>::isEqual(const BaseData& src) const {
	assert(dynamic_cast<const Data<T>*>(&src) != NULL);

	const Data<T>& srcData = dynamic_cast<const Data<T>&>(src);
	return value == srcData.value;
}

template<typename T>
void Datablock::Data<T>::toJson(io::json& j) const {
	j = value;
}

template<typename T>
void Datablock::Data<T>::fromJson(const io::json& j) {
	value = j.get<T>();
}

}
