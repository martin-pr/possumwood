#pragma once

#include "vbo.h"

#include <iostream>
#include <cassert>

#include "glsl_traits.h"
#include "buffer.inl"

namespace possumwood {

template <typename T>
VBO<T>::VBO(std::size_t vertexCount, std::size_t width)
    : m_vertexCount(vertexCount), m_width(width) {
}

template <typename T>
VBO<T>::~VBO() {
}

template <typename T>
void VBO<T>::init(Buffer<T>& buffer) {
	// just make sure everything is consistent
	assert(m_vertexCount == buffer.vertexCount());
	assert(m_width == buffer.width());

	// bind the buffer to work with
	glBindBuffer(GL_ARRAY_BUFFER, id());

	// synchronously transfer these, using raw pointer to the first element
	glBufferData(GL_ARRAY_BUFFER, buffer.vertexCount() * buffer.width() * sizeof(T),
	             buffer.element(0).ptr(), GL_STATIC_DRAW);

	// unbind the buffer to work with
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	setInitialised(true);
}

template <typename T>
std::size_t VBO<T>::width() const {
	return m_width;
}

namespace {
template <typename T>
struct VBOType {};

template <>
struct VBOType<float> {
	static constexpr GLenum type() {
		return GL_FLOAT;
	}
};

template <>
struct VBOType<double> {
	static constexpr GLenum type() {
		return GL_DOUBLE;
	}
};
}

template <typename T>
GLenum VBO<T>::type() const {
	return VBOType<T>::type();
}
}
