#pragma once

#include "buffer.h"

namespace possumwood {

template <typename T>
Buffer<T>::Buffer(std::size_t width, std::size_t arrSize, std::size_t vertexCount)
    : m_width(width), m_arraySize(arrSize), m_vertexCount(vertexCount),
      m_data(width * arrSize * vertexCount) {
}

template <typename T>
std::size_t Buffer<T>::vertexCount() const {
	return m_vertexCount;
}

template <typename T>
std::size_t Buffer<T>::arraySize() const {
	return m_arraySize;
}

template <typename T>
std::size_t Buffer<T>::width() const {
	return m_width;
}
}
