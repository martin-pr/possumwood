#pragma once

#include <vector>

#include <boost/noncopyable.hpp>

namespace possumwood {

/// A simple non-copyable raw-data container for OpenGL buffers.
template <typename T>
class Buffer : public boost::noncopyable {
  public:
	Buffer(std::size_t width, std::size_t arrSize, std::size_t vertexCount);

	std::size_t vertexCount() const;
	std::size_t arraySize() const;
	std::size_t width() const;

	/// A simple single-element assignable holder, providing a slightly nicer API
	class Element {
	  public:
		template <typename SRC>
		Element& operator=(const SRC& src) {
			for(auto it = m_begin; it != m_end; ++it)
				*it = src[it - m_begin];

			return *this;
		};

		Element& operator=(const float& src) {
			*m_begin = src;

			return *this;
		};

		T* ptr() {
			return &(*m_begin);
		}

	  private:
		Element(typename std::vector<T>::iterator begin,
		        typename std::vector<T>::iterator end)
		    : m_begin(begin), m_end(end) {
		}

		typename std::vector<T>::iterator m_begin, m_end;

		friend class Buffer;
	};

	Element element(std::size_t vertexIndex, std::size_t arrayIndex) {
		const std::size_t offset =
		    vertexIndex * m_arraySize * m_width + arrayIndex * m_width;
		assert(offset < m_data.size());

		auto it = m_data.begin() + offset;
		return Element(it, it + m_width);
	}

  protected:
  private:
	std::size_t m_width, m_arraySize, m_vertexCount;
	std::vector<T> m_data;
};
}
