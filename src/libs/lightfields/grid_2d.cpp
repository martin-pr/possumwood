#include "grid_2d.h"

#include "graph_2d.h"

#include <cassert>

namespace lightfields {

Grid2D::Edge::Edge(float capacity) : m_capacity(capacity), m_flow(0.0f) {
}

void Grid2D::Edge::setCapacity(const float& capacity) {
	assert(m_flow < Graph2D::flowEPS() && "set capacity only before the flow computation starts");
	m_capacity = capacity;
}

const float& Grid2D::Edge::capacity() const {
	return m_capacity;
}

void Grid2D::Edge::addFlow(const float& flow) {
	assert(flow >= -Graph2D::flowEPS());
	assert(flow + m_flow <= m_capacity + Graph2D::flowEPS() && "each edge can only carry its capacity");
	m_flow += flow;

	assert(m_flow >= Graph2D::flowEPS());
	assert(m_flow <= m_capacity + Graph2D::flowEPS());
}

const float& Grid2D::Edge::flow() const {
	return m_flow;
}

float Grid2D::Edge::residualCapacity() const {
	return m_capacity - m_flow;
}


/////////////

Grid2D::Grid2D(const Imath::V2i& size) : m_size(size), m_data(size[0]*size[1]) {
}

Grid2D::Edge& Grid2D::edge(const Imath::V2i& i) {
	assert(i[0] >= 0 && i[0] < m_size[0]);
	assert(i[1] >= 0 && i[1] < m_size[1]);

	const std::size_t index = i[0] + i[1] * m_size[0];
	assert(index < m_data.size());

	return m_data[index];
}

const Grid2D::Edge& Grid2D::edge(const Imath::V2i& i) const {
	assert(i[0] >= 0 && i[0] < m_size[0]);
	assert(i[1] >= 0 && i[1] < m_size[1]);

	const std::size_t index = i[0] + i[1] * m_size[0];
	assert(index < m_data.size());

	return m_data[index];
}

const Imath::V2i& Grid2D::size() const {
	return m_size;
}

}
