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
		NodesIterator() : m_recursive(false) {
		}

		NodesIterator(ITERATOR i, ITERATOR end, bool recursive) : m_recursive(recursive) {
			m_its.push(Item{i, end});
		}

		// corresponds to the interface of boost::indirect_iterator
		ITERATOR base() const {
			assert(!m_its.empty());
			return m_its.top().current;
		}

	private:
		struct Item {
			ITERATOR current;
			ITERATOR end;

			bool operator == (const Item& i) const {
				return current == i.current && end == i.end;
			}

			bool operator != (const Item& i) const {
				return current != i.current || end != i.end;
			}
		};

		std::stack<Item> m_its;
		bool m_recursive;

		friend class boost::iterator_core_access;

		void increment() {
			assert(!m_its.empty());
			assert(m_its.top().current != m_its.top().end);

			if(m_recursive) {
				// first, see if this is a network, and if so, iterate over sub-nodes
				if((*m_its.top().current)->template is<Network>() && !(*m_its.top().current)->template as<Network>().empty()) {
					m_its.push(Item{
						(*m_its.top().current)->template as<Network>().nodes().begin().base(),
						(*m_its.top().current)->template as<Network>().nodes().end().base()
					});
				}

				// current is not a Network instance - advance
				else {
					++m_its.top().current;
					while(m_its.top().current == m_its.top().end && m_its.size() > 1) {
						m_its.pop();

						assert(m_its.top().current != m_its.top().end);
						++m_its.top().current;
					}
				}
			}

			// non-recursive
			else {
				assert(m_its.size() == 1);
				++m_its.top().current;
			}
		}

		bool equal(const NodesIterator<ITERATOR>& other) const {
			return m_its == other.m_its;
		}

		typename ITERATOR::value_type::element_type& dereference() const {
			assert(!m_its.empty());
			assert(m_its.top().current != m_its.top().end);

			return **m_its.top().current;
		}
};

}
