#include "n_links.h"

#include <cassert>

#include "graph.h"

namespace lightfields {

NLinks::NLinks(const V2i& size, int n_link_value) : m_size(size), m_horiz((size.x-1) * size.y, Edge(n_link_value)), m_vert(size.x * (size.y - 1), Edge(n_link_value)) {
}

std::size_t NLinks::h_index(const V2i& i) const {
	return i.x + i.y * (m_size.x - 1);
}

std::size_t NLinks::v_index(const V2i& i) const {
	return i.x + i.y * m_size.x;
}

NLinks::Direction& NLinks::edge(const V2i& src, const V2i& dest) {
	assert((src.x - dest.x) * (src.x - dest.x) + (src.y - dest.y) * (src.y - dest.y) == 1);

	assert(src.x >= 0 && src.x < m_size.x);
	assert(dest.x >= 0 && dest.x < m_size.x);
	assert(src.y >= 0 && src.y < m_size.y);
	assert(src.y >= 0 && dest.y < m_size.y);

	if(src.x < dest.x)
		return m_horiz[h_index(src)].forward();

	if(src.x > dest.x)
		return m_horiz[h_index(dest)].backward();

	if(src.y < dest.y)
		return m_vert[v_index(src)].forward();

	if(src.y > dest.y)
		return m_vert[v_index(dest)].backward();

	assert(false && "bad edge index");
	throw(std::runtime_error("bad edge index"));
}

const NLinks::Direction& NLinks::edge(const V2i& src, const V2i& dest) const {
	assert((src.x - dest.x) * (src.x - dest.x) + (src.y - dest.y) * (src.y - dest.y) == 1);

	assert(src.x >= 0 && src.x < m_size.x);
	assert(dest.x >= 0 && dest.x < m_size.x);
	assert(src.y >= 0 && src.y < m_size.y);
	assert(src.y >= 0 && dest.y < m_size.y);

	if(src.x < dest.x)
		return m_horiz[h_index(src)].forward();

	if(src.x > dest.x)
		return m_horiz[h_index(dest)].backward();

	if(src.y < dest.y)
		return m_vert[v_index(src)].forward();

	if(src.y > dest.y)
		return m_vert[v_index(dest)].backward();

	assert(false && "bad edge index");

	assert(false && "bad edge index");
	throw(std::runtime_error("bad edge index"));
}

////////

NLinks::Edge::Edge(int capacity) : m_forward(this, true), m_backward(this, false), m_capacity(capacity), m_flow(0) {
}

NLinks::Edge::Edge(const Edge& e) : m_forward(this, true), m_backward(this, false), m_capacity(e.m_capacity), m_flow(e.m_flow) {
}

NLinks::Direction& NLinks::Edge::forward() {
	return m_forward;
}

const NLinks::Direction& NLinks::Edge::forward() const {
	return m_forward;
}

NLinks::Direction& NLinks::Edge::backward() {
	return m_backward;
}

const NLinks::Direction& NLinks::Edge::backward() const {
	return m_backward;
}

////////////////

NLinks::Direction::Direction(Edge* parent, bool forward) : m_parent(parent), m_forward(forward) {
}

int NLinks::Direction::capacity() const {
	return m_parent->m_capacity;
}

void NLinks::Direction::addFlow(const int& f) {
	assert(f > 0);
	assert(f <= residualCapacity());

	if(m_forward)
		m_parent->m_flow += f;
	else
		m_parent->m_flow -= f;
}

int NLinks::Direction::flow() const {
	if(m_forward)
		return std::max(m_parent->m_flow, 0);
	else
		return std::max(-m_parent->m_flow, 0);
}

int NLinks::Direction::residualCapacity() const {
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
