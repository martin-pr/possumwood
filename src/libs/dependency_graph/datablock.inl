#pragma once

#include "datablock.h"

#include <cassert>

#include "metadata.h"
#include "data.inl"

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

}
