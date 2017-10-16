#pragma once

#include "vbo.h"

#include <iostream>
#include <cassert>

#include "vbo_traits.h"

namespace possumwood {

template<typename T>
VBO<T>::VBO() {
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

	setInitialised(true);
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

	setInitialised(true);
}

template<typename T>
unsigned VBO<T>::width() const {
	return VBOTraits<T>::width();
}

template<typename T>
GLenum VBO<T>::type() const {
	return VBOTraits<T>::type();
}

}
