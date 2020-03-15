#include "link.h"

#include <cassert>
#include <algorithm>

namespace lightfields {

Link::Link(int capacity) : m_forward(this, true), m_backward(this, false), m_capacity(capacity), m_flow(0) {
}

Link::Link(const Link& e) : m_forward(this, true), m_backward(this, false), m_capacity(e.m_capacity), m_flow(e.m_flow) {
}

Link& Link::operator=(const Link& e) {
	m_capacity = e.m_capacity;
	m_flow = e.m_flow;

	return *this;
}

Link::Direction& Link::forward() {
	return m_forward;
}

const Link::Direction& Link::forward() const {
	return m_forward;
}

Link::Direction& Link::backward() {
	return m_backward;
}

const Link::Direction& Link::backward() const {
	return m_backward;
}

void Link::setCapacity(int c) {
	assert(m_flow == 0);
	assert(c >= 0);
	m_capacity = c;
}

////////////////

Link::Direction::Direction(Link* parent, bool forward) : m_parent(parent), m_forward(forward) {
}

int Link::Direction::capacity() const {
	return m_parent->m_capacity;
}

void Link::Direction::addFlow(const int& f) {
	assert(f > 0);
	assert(f <= residualCapacity());

	if(m_forward)
		m_parent->m_flow += f;
	else
		m_parent->m_flow -= f;
}

int Link::Direction::flow() const {
	if(m_forward)
		return std::max(m_parent->m_flow, 0);
	else
		return std::max(-m_parent->m_flow, 0);
}

int Link::Direction::residualCapacity() const {
	int result = 0;

	if(m_forward)
		result = m_parent->m_capacity - m_parent->m_flow;
	else
		result = m_parent->m_capacity + m_parent->m_flow;

	assert(result >= 0);
	assert(result <= 2 * m_parent->m_capacity);

	return result;
}

}
