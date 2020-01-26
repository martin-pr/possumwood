#pragma once

#include "datablock.h"

#include <cassert>

#include "metadata.h"
#include "data.inl"

namespace dependency_graph {

template<typename T>
const T& Datablock::get(size_t index) const {
	assert(index < m_data.size());

	const TypedData<T>& d = dynamic_cast<const TypedData<T>&>(data(index));
	return d.value;
}

template<typename T>
void Datablock::set(size_t index, const T& value) {
	assert(index < m_data.size());

	setData(index, TypedData<T>(value));
}

}
