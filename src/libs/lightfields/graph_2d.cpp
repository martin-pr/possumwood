#include "graph_2d.h"

#include <cassert>

#include "graph.h"

namespace lightfields {

Graph2D::Graph2D(const Imath::V2i& size, int n_link_value) : m_size(size), m_horiz((size[0]-1) * size[1], Edge(n_link_value)), m_vert(size[0] * (size[1] - 1), Edge(n_link_value)) {
}

std::size_t Graph2D::h_index(const Imath::V2i& i) const {
	return i[0] + i[1] * (m_size[0] - 1);
}

std::size_t Graph2D::v_index(const Imath::V2i& i) const {
	return i[0] + i[1] * m_size[0];
}

Graph2D::Direction& Graph2D::edge(const Imath::V2i& src, const Imath::V2i& dest) {
	assert((src[0] - dest[0]) * (src[0] - dest[0]) + (src[1] - dest[1]) * (src[1] - dest[1]) == 1);

	assert(src[0] >= 0 && src[0] < m_size[0]);
	assert(dest[0] >= 0 && dest[0] < m_size[0]);
	assert(src[1] >= 0 && src[1] < m_size[1]);
	assert(src[1] >= 0 && dest[1] < m_size[1]);

	if(src[0] < dest[0])
		return m_horiz[h_index(src)].forward();

	if(src[0] > dest[0])
		return m_horiz[h_index(dest)].backward();

	if(src[1] < dest[1])
		return m_vert[v_index(src)].forward();

	if(src[1] > dest[1])
		return m_vert[v_index(dest)].backward();

	assert(false && "bad edge index");
	throw(std::runtime_error("bad edge index"));
}

const Graph2D::Direction& Graph2D::edge(const Imath::V2i& src, const Imath::V2i& dest) const {
	assert((src[0] - dest[0]) * (src[0] - dest[0]) + (src[1] - dest[1]) * (src[1] - dest[1]) == 1);

	assert(src[0] >= 0 && src[0] < m_size[0]);
	assert(dest[0] >= 0 && dest[0] < m_size[0]);
	assert(src[1] >= 0 && src[1] < m_size[1]);
	assert(src[1] >= 0 && dest[1] < m_size[1]);

	if(src[0] < dest[0])
		return m_horiz[h_index(src)].forward();

	if(src[0] > dest[0])
		return m_horiz[h_index(dest)].backward();

	if(src[1] < dest[1])
		return m_vert[v_index(src)].forward();

	if(src[1] > dest[1])
		return m_vert[v_index(dest)].backward();

	assert(false && "bad edge index");

	assert(false && "bad edge index");
	throw(std::runtime_error("bad edge index"));
}

////////

Graph2D::Edge::Edge(int capacity) : m_forward(this, true), m_backward(this, false), m_capacity(capacity), m_flow(0) {
}

Graph2D::Edge::Edge(const Edge& e) : m_forward(this, true), m_backward(this, false), m_capacity(e.m_capacity), m_flow(e.m_flow) {
}

Graph2D::Direction& Graph2D::Edge::forward() {
	return m_forward;
}

const Graph2D::Direction& Graph2D::Edge::forward() const {
	return m_forward;
}

Graph2D::Direction& Graph2D::Edge::backward() {
	return m_backward;
}

const Graph2D::Direction& Graph2D::Edge::backward() const {
	return m_backward;
}

////////////////

Graph2D::Direction::Direction(Edge* parent, bool forward) : m_parent(parent), m_forward(forward) {
}

int Graph2D::Direction::capacity() const {
	return m_parent->m_capacity;
}

void Graph2D::Direction::addFlow(const int& f) {
	assert(f > 0);
	assert(f <= residualCapacity());

	if(m_forward)
		m_parent->m_flow += f;
	else
		m_parent->m_flow -= f;
}

int Graph2D::Direction::flow() const {
	if(m_forward)
		return std::max(m_parent->m_flow, 0);
	else
		return std::max(-m_parent->m_flow, 0);
}

int Graph2D::Direction::residualCapacity() const {
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
