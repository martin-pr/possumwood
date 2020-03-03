#include "grid_2d.h"

#include "graph.h"

#include <cassert>

namespace lightfields {

Grid2D::Edge::Edge(int capacity) : m_capacity(capacity), m_flow(0.0f) {
}

void Grid2D::Edge::setCapacity(const int& capacity) {
	assert(m_flow == 0 && "set capacity only before the flow computation starts");
	m_capacity = capacity;
}

const int& Grid2D::Edge::capacity() const {
	return m_capacity;
}

void Grid2D::Edge::addFlow(const int& flow) {
	assert(flow > 0);
	assert(flow + m_flow <= m_capacity && "each edge can only carry its capacity");
	m_flow += flow;

	assert(m_flow >= 0);
	assert(m_flow <= m_capacity);
}

const int& Grid2D::Edge::flow() const {
	return m_flow;
}

int Grid2D::Edge::residualCapacity() const {
	return m_capacity - m_flow;
}


/////////////

Grid2D::Grid2D(const V2i& size) : m_size(size), m_data(size.x*size.y) {
}

Grid2D::Edge& Grid2D::edge(const V2i& i) {
	assert(i.x >= 0 && i.x < m_size.x);
	assert(i.y >= 0 && i.y < m_size.y);

	const std::size_t index = i.x + i.y * m_size.x;
	assert(index < m_data.size());

	return m_data[index];
}

const Grid2D::Edge& Grid2D::edge(const V2i& i) const {
	assert(i.x >= 0 && i.x < m_size.x);
	assert(i.y >= 0 && i.y < m_size.y);

	const std::size_t index = i.x + i.y * m_size.x;
	assert(index < m_data.size());

	return m_data[index];
}

const V2i& Grid2D::size() const {
	return m_size;
}

}
