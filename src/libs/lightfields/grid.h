#pragma once

#include <cassert>
#include <type_traits>
#include <vector>

namespace lightfields {

/// A trivial grid abstraction (not std-compliant, no need for now)
template <typename VALUE, typename CONTAINER = std::vector<VALUE>>
class Grid {
  public:
	explicit Grid(unsigned rows, unsigned cols, const VALUE& value)
	    : m_rows(rows), m_cols(cols), m_p(rows * cols, value) {
	}

	explicit Grid(unsigned rows, unsigned cols) : m_rows(rows), m_cols(cols), m_p(rows * cols) {
	}

	decltype(std::declval<CONTAINER>()[0]) operator()(unsigned row, unsigned col) {
		assert(row < m_rows && col < m_cols);
		return m_p[row * m_cols + col];
	}

	decltype(std::declval<const CONTAINER>()[0]) operator()(unsigned row, unsigned col) const {
		assert(row < m_rows && col < m_cols);
		return m_p[row * m_cols + col];
	}

	unsigned rows() const {
		return m_rows;
	}

	unsigned cols() const {
		return m_cols;
	}

	void swap(Grid& g) {
		std::swap(g.m_rows, m_rows);
		std::swap(g.m_cols, m_cols);

		m_p.swap(g.m_p);
	}

	const CONTAINER& container() const {
		return m_p;
	}

	CONTAINER& container() {
		return m_p;
	}

  private:
	unsigned m_rows, m_cols;
	CONTAINER m_p;
};

}  // namespace lightfields
