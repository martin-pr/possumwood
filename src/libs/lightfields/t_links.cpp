#include "t_links.h"

#include "graph.h"

#include <cassert>

namespace lightfields {

TLinks::Edge::Edge(int capacity) : m_capacity(capacity), m_flow(0) {
}

void TLinks::Edge::setCapacity(const int& capacity) {
	assert(m_flow == 0 && "set capacity only before the flow computation starts");
	m_capacity = capacity;
}

const int& TLinks::Edge::capacity() const {
	return m_capacity;
}

void TLinks::Edge::addFlow(const int& flow) {
	assert(flow > 0);
	assert(flow + m_flow <= m_capacity && "each edge can only carry its capacity");
	m_flow += flow;

	assert(m_flow >= 0);
	assert(m_flow <= m_capacity);
	assert(residualCapacity() >= 0);
}

const int& TLinks::Edge::flow() const {
	return m_flow;
}

int TLinks::Edge::residualCapacity() const {
	return m_capacity - m_flow;
}


/////////////

TLinks::TLinks(const V2i& size) : m_size(size), m_data(size.x*size.y) {
}

TLinks::Edge& TLinks::edge(const V2i& i) {
	assert(i.x >= 0 && i.x < m_size.x);
	assert(i.y >= 0 && i.y < m_size.y);

	const std::size_t index = i.x + i.y * m_size.x;
	assert(index < m_data.size());

	return m_data[index];
}

const TLinks::Edge& TLinks::edge(const V2i& i) const {
	assert(i.x >= 0 && i.x < m_size.x);
	assert(i.y >= 0 && i.y < m_size.y);

	const std::size_t index = i.x + i.y * m_size.x;
	assert(index < m_data.size());

	return m_data[index];
}

const V2i& TLinks::size() const {
	return m_size;
}

}
