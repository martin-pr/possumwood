#include "link.h"

#include <cassert>
#include <algorithm>

namespace lightfields {

Link::Link(int capacity) : m_forward(this, true), m_backward(this, false), m_forwardCapacity(capacity), m_backwardCapacity(capacity), m_flow(0) {
}

Link::Link(int forwardCapacity, int backwardCapacity) : m_forward(this, true), m_backward(this, false), m_forwardCapacity(forwardCapacity), m_backwardCapacity(backwardCapacity), m_flow(0) {

}


Link::Link(const Link& e) : m_forward(this, true), m_backward(this, false), m_forwardCapacity(e.m_forwardCapacity), m_backwardCapacity(e.m_backwardCapacity), m_flow(e.m_flow) {
}

Link& Link::operator=(const Link& e) {
	m_forwardCapacity = e.m_forwardCapacity;
	m_backwardCapacity = e.m_backwardCapacity;
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
	m_forwardCapacity = c;
	m_backwardCapacity = c;
}

void Link::setCapacity(int forward, int backward) {
	assert(m_flow == 0);
	assert(forward >= 0);
	assert(backward >= 0);
	m_forwardCapacity = forward;
	m_backwardCapacity = backward;
}

////////////////

Link::Direction::Direction(Link* parent, bool forward) : m_parent(parent), m_forward(forward) {
}

int Link::Direction::capacity() const {
	if(m_forward)
		return m_parent->m_forwardCapacity;
	return m_parent->m_backwardCapacity;
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
		result = (m_parent->m_forwardCapacity + m_parent->m_backwardCapacity) - (m_parent->m_flow + m_parent->m_backwardCapacity);
	else
		result = (m_parent->m_forwardCapacity + m_parent->m_backwardCapacity) - (-m_parent->m_flow + m_parent->m_forwardCapacity);

	assert(result >= 0);
	assert(result <= m_parent->m_forwardCapacity + m_parent->m_backwardCapacity);

	return result;
}

}
