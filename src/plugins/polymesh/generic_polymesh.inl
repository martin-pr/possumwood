#pragma once

#include "generic_polymesh.h"

#include <cassert>

#include "generic_container.inl"

namespace possumwood {
namespace polymesh {

template<typename T>
const T& GenericPolymesh::Vertex::get(const GenericContainer<Vertex>::Handle& handle) const {
	return m_parent->get<T>(handle, m_index);
}

template<typename T>
T& GenericPolymesh::Vertex::get(const GenericContainer<Vertex>::Handle& handle) {
	return m_parent->get<T>(handle, m_index);
}

template<typename T>
void GenericPolymesh::Vertex::set(const GenericContainer<Vertex>::Handle& handle, const T& value) {
	m_parent->get<T>(handle, m_index) = value;
}

template<typename T>
const T& GenericPolymesh::Index::get(const GenericContainer<Index>::Handle& handle) const {
	return m_parent->get<T>(handle, m_index);
}

template<typename T>
T& GenericPolymesh::Index::get(const GenericContainer<Index>::Handle& handle) {
	return m_parent->get<T>(handle, m_index);
}

template<typename T>
void GenericPolymesh::Index::set(const GenericContainer<Index>::Handle& handle, const T& value) {
	m_parent->get<T>(handle, m_index) = value;
}

template<typename T>
const T& GenericPolymesh::Polygon::get(const GenericContainer<Polygon>::Handle& handle) const {
	return m_parent->m_polygons.get<T>(handle, m_index);
}

template<typename T>
T& GenericPolymesh::Polygon::get(const GenericContainer<Polygon>::Handle& handle) {
	return m_parent->m_polygons.get<T>(handle, m_index);
}

template<typename T>
void GenericPolymesh::Polygon::set(const GenericContainer<Polygon>::Handle& handle, const T& value) {
	m_parent->m_polygons.get<T>(handle, m_index) = value;
}

template<typename ITER>
GenericPolymesh::Polygons::iterator GenericPolymesh::Polygons::add(ITER start, ITER stop) {
	// add the start pointer
	m_parent->m_polygonPointers.push_back(m_parent->m_vertexIndices.size());
	m_parent->m_polygons.add();

	// add all the indices
	for(auto i = start; i != stop; ++i) {
		m_parent->m_vertexIndices.push_back(*i);
		m_parent->m_indices.add();

		assert(m_parent->m_vertexIndices.size() == m_parent->m_indices.size());
		assert(*i < m_parent->vertices().size());
	}

	// and make a new iterator
	return begin() + (m_parent->m_polygonPointers.size()-1);
}


}
}
