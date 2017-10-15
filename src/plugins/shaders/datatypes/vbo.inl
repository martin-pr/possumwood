#pragma once

#include "vbo.h"

#include <iostream>
#include <cassert>

#include "vbo_traits.h"

namespace possumwood {

template<typename T>
VBO<T>::VBO() : m_initialised(false) {
}

template<typename T>
VBO<T>::~VBO() {
}

template<typename T>
template<typename ITERATOR>
void VBO<T>::init(ITERATOR begin, ITERATOR end) {
	// bind the buffer to work with
	glBindBuffer(GL_ARRAY_BUFFER, id());

	// build a vector to hold the data
	std::vector<T> data;
	while(begin != end) {
		data.push_back(*begin);
		++begin;
	}

	// synchronously transfer these
	glBufferData(GL_ARRAY_BUFFER, sizeof(T)*data.size(), &data[0], GL_STATIC_DRAW);

	// unbind the buffer to work with
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_initialised = true;
}

/// builds a VBO out of an initializer list
template<typename T>
void VBO<T>::init(std::initializer_list<T> l) {
	// bind the buffer to work with
	glBindBuffer(GL_ARRAY_BUFFER, id());

	// build a vector to hold the data
	std::vector<T> data(l);

	// synchronously transfer these
	glBufferData(GL_ARRAY_BUFFER, sizeof(T)*data.size(), &data[0], GL_STATIC_DRAW);

	// unbind the buffer to work with
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_initialised = true;
}

template<typename T>
void VBO<T>::use(GLint attribLocation) const {
	assert(attribLocation >= 0);

	glBindBuffer(GL_ARRAY_BUFFER, id());

	glEnableVertexAttribArray(attribLocation);
	glVertexAttribPointer(attribLocation, VBOTraits<T>::width(), VBOTraits<T>::type(), 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template<typename T>
bool VBO<T>::isInitialised() const {
	return m_initialised;
}

}
