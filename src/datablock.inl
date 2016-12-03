#pragma once

#include "datablock.h"

#include <cassert>

#include "metadata.h"

template<typename T>
const T& Datablock::get(const InAttr<T>& attr) const {
	assert(attr.isValid());
	assert(attr.offset() < m_data.size());

	const Data<T>& data = dynamic_cast<const Data<T>&>(*m_data[attr.offset()]);
	return data.value;
}

template<typename T>
void Datablock::set(const OutAttr<T>& attr, const T& value) {
	assert(attr.isValid());
	assert(attr.offset() < m_data.size());

	Data<T>& data = dynamic_cast<Data<T>&>(*m_data[attr.offset()]);
	data.value = value;
}

template<typename T>
void Datablock::set(const InAttr<T>& attr, const T& value) {
	assert(attr.isValid());
	assert(attr.offset() < m_data.size());

	Data<T>& data = dynamic_cast<Data<T>&>(*m_data[attr.offset()]);
	data.value = value;
}
