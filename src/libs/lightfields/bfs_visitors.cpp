#include "bfs_visitors.h"

#include <limits>
#include <cassert>
#include <iostream>

namespace lightfields {

#ifndef BFS_VISITORS_TRIVIAL

unsigned BFSVisitors::vec2index(const Index& v) const {
	if(v.pos.x == -1 && v.pos.y == -1)
		return std::numeric_limits<unsigned>::max()-1;

	assert(v.pos.x >= 0 && v.pos.x < (int)m_size.x);
	assert(v.pos.y >= 0 && v.pos.y < (int)m_size.y);
	assert(v.n_layer < m_layerCount);

	const unsigned index = unsigned(v.pos.x) +
		unsigned(v.pos.y) * m_size.x + v.n_layer * m_layerSize;
	assert(index < m_values.size());

	return index;
}

Index BFSVisitors::index2vec(unsigned i) const {
	if(i == std::numeric_limits<unsigned>::max()-1)
		return Index{V2i(-1, -1), 0};

	assert(i < m_values.size());

	const unsigned layer = (unsigned)i / m_layerSize;
	const int ii = i % m_layerSize;

	return Index{V2i(ii % m_size.x, ii / m_size.x), layer};
}

BFSVisitors::BFSVisitors(const V2i& size, unsigned layerCount) : m_size(size.x, size.y),
	m_layerCount(layerCount), m_layerSize(size.x * size.y),
	m_values(m_size.x*m_size.y*layerCount, std::numeric_limits<unsigned>::max())
{
}

bool BFSVisitors::visited(const Index& v) const {
	const unsigned index = vec2index(v);

	assert(index < m_values.size());
	return m_values[index] != std::numeric_limits<unsigned>::max();
}

Index BFSVisitors::parent(const Index& v) const {
	const unsigned index = vec2index(v);

	assert(visited(v));

	return index2vec(m_values[index]);
}

void BFSVisitors::visit(const Index& index, const Index& parent) {
	assert(!visited(index));

	const unsigned parent_id = vec2index(parent);
	assert(parent_id == std::numeric_limits<unsigned>::max()-1 || parent_id < m_values.size());

	m_values[vec2index(index)] = parent_id;

	assert(visited(index));
	assert(this->parent(index) == parent);
}

///////////////

#else

BFSVisitors::BFSVisitors(const V2i& size, unsigned layer_count) {
}

bool BFSVisitors::visited(const Index& index) const {
	return m_visited.find(index) != m_visited.end();
}

Index BFSVisitors::parent(const Index& index) const {
	assert(visited(index));
	return m_visited.find(index)->second;
}

void BFSVisitors::visit(const Index& index, const Index& parent) {
	m_visited[index] = parent;
}

void BFSVisitors::clear() {
	m_visited.clear();
}

#endif

}
