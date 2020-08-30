#pragma once

#include <cassert>
#include <iostream>

#include "buffer.inl"
#include "glsl_traits.h"
#include "vbo.h"

namespace possumwood {

template <typename T>
VBO<T>::VBO(std::size_t vertexCount) : m_vertexCount(vertexCount) {
}

template <typename T>
VBO<T>::~VBO() {
}

template <typename T>
void VBO<T>::init(Buffer<typename VBOTraits<T>::element>& buffer) {
	// just make sure everything is consistent
	assert(m_vertexCount == buffer.vertexCount());

	assert(VBOTraits<T>::width() == buffer.width());

	// bind the buffer to work with
	glBindBuffer(GL_ARRAY_BUFFER, id());

	// synchronously transfer these, using raw pointer to the first element
	glBufferData(GL_ARRAY_BUFFER, buffer.vertexCount() * buffer.width() * sizeof(typename VBOTraits<T>::element),
	             buffer.element(0).ptr(), GL_STATIC_DRAW);

	// unbind the buffer to work with
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	setInitialised(true);
}

template <typename T>
std::size_t VBO<T>::width() const {
	return VBOTraits<T>::width();
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

template <>
struct VBOType<int> {
	static constexpr GLenum type() {
		return GL_INT;
	}
};
}  // namespace

template <typename T>
GLenum VBO<T>::type() const {
	return VBOType<typename VBOTraits<T>::element>::type();
}
}  // namespace possumwood
