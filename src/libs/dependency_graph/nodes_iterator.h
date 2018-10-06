#pragma once

#include <stack>

#include <boost/iterator/iterator_facade.hpp>

namespace dependency_graph {

class Network;

template<typename ITERATOR>
class NodesIterator : public boost::iterator_facade <
	NodesIterator<ITERATOR>,
	typename ITERATOR::value_type::element_type, // element type stored in an std smart pointer
	boost::forward_traversal_tag > {

	public:
		NodesIterator();
		NodesIterator(ITERATOR i, ITERATOR end, bool recursive);

		// corresponds to the interface of boost::indirect_iterator
		ITERATOR base() const;

	private:
		struct Item {
			ITERATOR current;
			ITERATOR end;

			bool operator == (const Item& i) const;

			bool operator != (const Item& i) const;
		};

		std::stack<Item> m_its;
		bool m_recursive;

		friend class boost::iterator_core_access;

		void increment();
		bool equal(const NodesIterator<ITERATOR>& other) const;
		typename ITERATOR::value_type::element_type& dereference() const;
};

}
