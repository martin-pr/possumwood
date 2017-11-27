#pragma once

#include "vbo.h"

#include <iostream>
#include <cassert>

#include "glsl_traits.h"
#include "buffer.inl"

namespace possumwood {

template<typename T, std::size_t WIDTH>
VBO<T, WIDTH>::VBO(std::size_t arraySize, std::size_t vertexCount) : m_arraySize(arraySize), m_vertexCount(vertexCount) {
}

template<typename T, std::size_t WIDTH>
VBO<T, WIDTH>::~VBO() {
}

template<typename T, std::size_t WIDTH>
void VBO<T, WIDTH>::init(Buffer<T, WIDTH>& buffer) {
	// bind the buffer to work with
	glBindBuffer(GL_ARRAY_BUFFER, id());

	// synchronously transfer these
	glBufferData(GL_ARRAY_BUFFER, buffer.size()*buffer.arraySize()*WIDTH*sizeof(T), &(buffer[0][0][0]), GL_STATIC_DRAW);

	// unbind the buffer to work with
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	setInitialised(true);
}

template<typename T, std::size_t WIDTH>
std::size_t VBO<T, WIDTH>::arraySize() const {
	return m_arraySize;
}

template<typename T, std::size_t WIDTH>
std::size_t VBO<T, WIDTH>::width() const {
	return WIDTH;
}

template<typename T, std::size_t WIDTH>
GLenum VBO<T, WIDTH>::type() const {
	return GLSLTraits<std::array<T, WIDTH>>::type();
}

}
