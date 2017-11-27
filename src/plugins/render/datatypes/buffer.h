#pragma once

#include <vector>
#include <array>

#include <boost/noncopyable.hpp>
// #include <boost/range/adaptor/strided.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>

namespace possumwood {

/// A simple non-cppyable raw-data container for OpenGL buffers.
template <typename T, std::size_t WIDTH>
class Buffer : public boost::noncopyable {
  public:
	/// single item class, assignable from anything that has [] operator
	class Item {
	  public:
		constexpr std::size_t size() const {
			return WIDTH;
		}

		const T& operator[](std::size_t index) const;
		T& operator[](std::size_t index);

		typedef typename std::array<T, WIDTH>::iterator iterator;
		iterator begin();
		iterator end();

		typedef typename std::array<T, WIDTH>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		/// a silly assignment operator, that doesn't require anything more than []
		/// operator from the VECT class
		template <typename VECT>
		Item& operator=(const VECT& i);

	  private:
		std::array<T, WIDTH> m_data;
	};

	/// Buffer's iterator, dereference returns a Row instance - NEEDS const_iterator
	template <typename BASE, typename RESULT>
	class iterator_base
	    : public boost::iterator_facade<iterator_base<BASE, RESULT>, RESULT,
	                                    boost::random_access_traversal_tag, RESULT> {
	  public:
		iterator_base();

	  private:
		iterator_base(BASE it, std::size_t rowSize);

		bool equal(const iterator_base& other) const;
		RESULT dereference();
		const RESULT dereference() const;
		void increment();

		BASE m_it;
		std::size_t m_rowSize;

		friend class Buffer;
		friend class boost::iterator_core_access;
	};

	/// A single row item - NEEDS REFACTORING
	class Row {
	  public:
		typedef typename std::vector<Item>::iterator iterator;
		iterator begin();
		iterator end();

		typedef typename std::vector<Item>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		Item& operator[](std::size_t index);

		std::size_t size() const;

	  private:
		Row(typename std::vector<Item>::iterator it, std::size_t rowSize);

		typename std::vector<Item>::iterator m_it;
		std::size_t m_rowSize;

		friend class Buffer::iterator;
		friend class Buffer;
	};

	Buffer(std::size_t arrSize, std::size_t vertexCount);

	/// returns the number of vertices this buffer is made for
	std::size_t size() const;
	std::size_t arraySize() const;
	std::size_t width() const;

	typedef iterator_base<typename std::vector<Item>::iterator, Row> iterator;
	iterator begin();
	iterator end();

	typedef iterator_base<typename std::vector<Item>::const_iterator, const Row> const_iterator;
	const_iterator begin() const;
	const_iterator end() const;

	Row operator[](std::size_t index);
	const Row operator[](std::size_t index) const;

	const T* rawBegin() const;
	const T* rawEnd() const;

  protected:
  private:
	std::size_t m_rowSize;
	std::vector<Item> m_data;

	friend class VertexData;
};
}
