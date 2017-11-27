#pragma once

#include "buffer.h"

#include <iostream>

namespace possumwood {

template <typename T, std::size_t WIDTH>
Buffer<T, WIDTH>::Buffer(std::size_t arrSize, std::size_t vertexCount)
    : m_rowSize(arrSize), m_data(vertexCount * arrSize) {
}

template <typename T, std::size_t WIDTH>
std::size_t Buffer<T, WIDTH>::size() const {
	return m_data.size() / m_rowSize;
}

template<typename T, std::size_t WIDTH>
std::size_t Buffer<T, WIDTH>::arraySize() const {
	return m_rowSize;
}

template<typename T, std::size_t WIDTH>
std::size_t Buffer<T, WIDTH>::width() const {
	return WIDTH;
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::iterator Buffer<T, WIDTH>::begin() {
	return iterator(m_data.begin(), m_rowSize);
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::iterator Buffer<T, WIDTH>::end() {
	return iterator(m_data.end(), m_rowSize);
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::const_iterator Buffer<T, WIDTH>::begin() const {
	return const_iterator(m_data.begin(), m_rowSize);
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::const_iterator Buffer<T, WIDTH>::end() const {
	return const_iterator(m_data.end(), m_rowSize);
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::Row Buffer<T, WIDTH>::operator[](std::size_t index) {
	assert(index < m_data.size() / m_rowSize);
	return Row(m_data.begin() + index*m_rowSize, m_rowSize);
}

template <typename T, std::size_t WIDTH>
const typename Buffer<T, WIDTH>::Row Buffer<T, WIDTH>::operator[](std::size_t index) const {
	assert(index < m_data.size() / m_rowSize);
	return Row(m_data.begin() + index*m_rowSize, m_rowSize);
}

template <typename T, std::size_t WIDTH>
const T* Buffer<T, WIDTH>::rawBegin() const {
	return &((*m_data.begin())[0]);
}

template <typename T, std::size_t WIDTH>
const T* Buffer<T, WIDTH>::rawEnd() const {
	return &((*m_data.end())[0]);
}


////////////////////////////

template <typename T, std::size_t WIDTH>
const T& Buffer<T, WIDTH>::Item::operator[](std::size_t index) const {
	assert(index < WIDTH);
	return m_data[index];
}

template <typename T, std::size_t WIDTH>
T& Buffer<T, WIDTH>::Item::operator[](std::size_t index) {
	assert(index < WIDTH);
	return m_data[index];
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::Item::iterator Buffer<T, WIDTH>::Item::begin() {
	return m_data.begin();
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::Item::iterator Buffer<T, WIDTH>::Item::end() {
	return m_data.end();
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::Item::const_iterator Buffer<T, WIDTH>::Item::begin() const {
	return m_data.begin();
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::Item::const_iterator Buffer<T, WIDTH>::Item::end() const {
	return m_data.end();
}

template <typename T, std::size_t WIDTH>
template <typename VECT>
typename Buffer<T, WIDTH>::Item& Buffer<T, WIDTH>::Item::operator=(const VECT& vect) {
	for(std::size_t i = 0; i < WIDTH; ++i)
		m_data[i] = vect[i];

	return *this;
}

///////////////////////

template <typename T, std::size_t WIDTH>
Buffer<T, WIDTH>::Row::Row(typename std::vector<Item>::iterator it, std::size_t rowSize) : m_it(it), m_rowSize(rowSize) {

}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::Row::iterator Buffer<T, WIDTH>::Row::begin() {
	return m_it;
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::Row::iterator Buffer<T, WIDTH>::Row::end() {
	return m_it + m_rowSize;
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::Row::const_iterator Buffer<T, WIDTH>::Row::begin() const {
	return m_it;
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::Row::const_iterator Buffer<T, WIDTH>::Row::end() const {
	return m_it + m_rowSize;
}

template <typename T, std::size_t WIDTH>
typename Buffer<T, WIDTH>::Item& Buffer<T, WIDTH>::Row::operator[](std::size_t index) {
	assert(index < m_rowSize);
	return *(m_it + index);
}

template <typename T, std::size_t WIDTH>
std::size_t Buffer<T, WIDTH>::Row::size() const {
	return m_rowSize;
}

//////////////////////

template <typename T, std::size_t WIDTH>
template <typename BASE, typename RESULT>
Buffer<T, WIDTH>::iterator_base<BASE, RESULT>::iterator_base() {
}

template <typename T, std::size_t WIDTH>
template <typename BASE, typename RESULT>
Buffer<T, WIDTH>::iterator_base<BASE, RESULT>::iterator_base(BASE it, std::size_t rowSize) : m_it(it), m_rowSize(rowSize) {

}

template <typename T, std::size_t WIDTH>
template <typename BASE, typename RESULT>
bool Buffer<T, WIDTH>::iterator_base<BASE, RESULT>::equal(const iterator_base<BASE, RESULT>& other) const {
	return m_it == other.m_it;
}

template <typename T, std::size_t WIDTH>
template <typename BASE, typename RESULT>
RESULT Buffer<T, WIDTH>::iterator_base<BASE, RESULT>::dereference() {
	return Row(m_it, m_rowSize);
}

template <typename T, std::size_t WIDTH>
template <typename BASE, typename RESULT>
const RESULT Buffer<T, WIDTH>::iterator_base<BASE, RESULT>::dereference() const {
	return Row(m_it, m_rowSize);
}

template <typename T, std::size_t WIDTH>
template <typename BASE, typename RESULT>
void Buffer<T, WIDTH>::iterator_base<BASE, RESULT>::increment() {
	m_it += m_rowSize;
}

}
