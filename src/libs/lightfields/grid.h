#pragma once

#include <cassert>
#include <vector>

namespace lightfields {

/// A trivial grid abstraction (not std-compliant, no need for now)
template<typename VALUE>
class Grid {
	public:
		Grid(unsigned rows, unsigned cols, const VALUE& value = VALUE()) : m_rows(rows), m_cols(cols), m_p(rows*cols, value) {
		}

		VALUE& operator() (unsigned row, unsigned col) {
			assert(row < m_rows && col < m_cols);
			return m_p[row * m_cols + col];
		}

		const VALUE& operator() (unsigned row, unsigned col) const {
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

	private:
		unsigned m_rows, m_cols;
		std::vector<VALUE> m_p;
};

}
