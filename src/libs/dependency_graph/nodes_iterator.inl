#pragma once

#include "node_base.inl"
#include "nodes_iterator.h"

namespace dependency_graph {

template <typename ITERATOR>
NodesIterator<ITERATOR>::NodesIterator() : m_recursive(false) {
}

template <typename ITERATOR>
NodesIterator<ITERATOR>::NodesIterator(ITERATOR i, ITERATOR end, bool recursive) : m_recursive(recursive) {
	m_its.push(Item{i, end});
}

template <typename ITERATOR>
ITERATOR NodesIterator<ITERATOR>::NodesIterator::base() const {
	assert(!m_its.empty());
	return m_its.top().current;
}

template <typename ITERATOR>
bool NodesIterator<ITERATOR>::NodesIterator::Item::operator==(const Item& i) const {
	return current == i.current && end == i.end;
}

template <typename ITERATOR>
bool NodesIterator<ITERATOR>::NodesIterator::Item::operator!=(const Item& i) const {
	return current != i.current || end != i.end;
}

template <typename ITERATOR>
void NodesIterator<ITERATOR>::increment() {
	assert(!m_its.empty());
	assert(m_its.top().current != m_its.top().end);

	if(m_recursive) {
		// first, see if this is a network, and if so, iterate over sub-nodes
		if((*m_its.top().current)->template is<Network>() && !(*m_its.top().current)->template as<Network>().empty()) {
			m_its.push(Item{(*m_its.top().current)->template as<Network>().nodes().begin().base(),
			                (*m_its.top().current)->template as<Network>().nodes().end().base()});
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

template <typename ITERATOR>
bool NodesIterator<ITERATOR>::equal(const NodesIterator<ITERATOR>& other) const {
	return m_its == other.m_its;
}

template <typename ITERATOR>
typename ITERATOR::value_type::element_type& NodesIterator<ITERATOR>::dereference() const {
	assert(!m_its.empty());
	assert(m_its.top().current != m_its.top().end);

	return **m_its.top().current;
}
}  // namespace dependency_graph
