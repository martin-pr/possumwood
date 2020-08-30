#pragma once

#include <cassert>

#include "data.inl"
#include "datablock.h"
#include "metadata.h"

namespace dependency_graph {

template <typename T>
const T& Datablock::get(size_t index) const {
	assert(index < m_data.size());

	return data(index).get<T>();
}

template <typename T>
void Datablock::set(size_t index, T value) {
	assert(index < m_data.size());

	setData(index, Data(std::move(value)));
}

}  // namespace dependency_graph
