#pragma once

# include <boost/iterator/iterator_facade.hpp>

namespace dependency_graph {

template<typename ITERATOR>
class NodesIterator : public boost::iterator_facade <
	NodesIterator<ITERATOR>,
	typename ITERATOR::value_type::element_type, // element type stored in an std smart pointer
	boost::forward_traversal_tag > {

	public:
		NodesIterator(ITERATOR i = ITERATOR()) : m_it(i) {
		}

		// corresponds to the interface of boost::indirect_iterator
		ITERATOR base() const {
			return m_it;
		}

	private:
		ITERATOR m_it;

		friend class boost::iterator_core_access;

		void increment() {
			++m_it;
		}

		bool equal(const NodesIterator<ITERATOR>& other) const
		{
			return m_it == other.m_it;
		}

		typename ITERATOR::value_type::element_type& dereference() const {
			return **m_it;
		}
};

}
