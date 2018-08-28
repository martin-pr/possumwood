#pragma once

#include <cassert>

#include "generic_base.h"

namespace possumwood {
namespace polymesh {

template<typename T>
const GenericBase::Handle& GenericBase::Handles::add(const std::string& name, const T& defaultValue) {
	auto it = m_handles.insert(Handle(name, typeid(T), m_handles.size()));

	std::unique_ptr<ArrayBase> arr(new Array<T>(defaultValue));
	arr->resize(m_parent->size());
	m_parent->m_data.push_back(std::move(arr));

	return *it.first;
}

template<typename T>
const T& GenericBase::get(const Handle& h, std::size_t index) const {
	assert(std::type_index(typeid(T)) == h.type());
	assert(h.m_index < m_data.size());
	assert(m_data[h.m_index]->type() == h.type());
	assert(m_data[h.m_index]->size() > index);

	const Array<T>& container = dynamic_cast<const Array<T>&>(*m_data[h.m_index]);
	return container[index];
}

template<typename T>
T& GenericBase::get(const Handle& h, std::size_t index) {
	assert(std::type_index(typeid(T)) == h.type());
	assert(h.m_index < m_data.size());
	assert(m_data[h.m_index]->type() == h.type());
	assert(m_data[h.m_index]->size() > index);

	Array<T>& container = dynamic_cast<Array<T>&>(*m_data[h.m_index]);
	return container[index];
}

}
}
