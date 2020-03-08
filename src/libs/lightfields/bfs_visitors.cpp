#include "bfs_visitors.h"

#include <limits>
#include <cassert>
#include <iostream>

namespace lightfields {

std::size_t BFSVisitors::vec2index(const Index& v) const {
	if(v.pos.x == -1 && v.pos.y == -1)
		return std::numeric_limits<std::size_t>::max();

	assert(v.pos.x >= 0 && v.pos.x < m_size.x);
	assert(v.pos.y >= 0 && v.pos.y < m_size.y);
	assert(v.n_layer < m_layerCount);

	const std::size_t index = v.pos.x + v.pos.y * m_size.x + v.n_layer * m_layerSize;
	assert(index < m_values.size());

	return index;
}

Index BFSVisitors::index2vec(std::size_t i) const {
	assert(i < m_values.size());

	const std::size_t layer = i / m_layerSize;
	i = i % m_layerSize;

	return Index{V2i(i % m_size.x, i / m_size.x), layer};
}

BFSVisitors::BFSVisitors(const V2i& size, std::size_t layerCount) : m_size(size), m_layerCount(layerCount), m_layerSize(size.x * size.y), m_stage(0), m_mask(std::numeric_limits<std::size_t>::max()), m_shift(0), m_values(m_size.x*m_size.y*layerCount) {
	std::size_t count = m_size.x * m_size.y * m_layerCount;
	while(count > 0) {
		count = count >> 1;
		m_mask = m_mask << 1;
		++m_shift;
	}

	m_stage = 1 << m_shift;

	assert(m_mask == (std::numeric_limits<std::size_t>::max() << m_shift));
}

bool BFSVisitors::visited(const Index& v) const {
	const std::size_t index = vec2index(v);

	assert(index < m_values.size());
	return (m_values[index] & m_mask) == m_stage;
}

Index BFSVisitors::parent(const Index& v) const {
	const std::size_t index = vec2index(v);

	assert(visited(v));

	if((m_values[index] | m_mask) == std::numeric_limits<std::size_t>::max())
		return Index{V2i(-1, -1), 0};

	return index2vec(m_values[index] & ~m_mask);
}

void BFSVisitors::visit(const Index& index, const Index& parent) {
	assert(!visited(index));

	const std::size_t parent_id = vec2index(parent);
	assert(parent_id == std::numeric_limits<std::size_t>::max() || parent_id < m_values.size());

	m_values[vec2index(index)] = (parent_id & ~m_mask) | m_stage;

	assert(visited(index));
	assert(this->parent(index) == parent);
}

void BFSVisitors::clear() {
	if((m_stage | ~m_mask) == std::numeric_limits<std::size_t>::max()) {
		m_stage = 1 << m_shift;
		std::fill(m_values.begin(), m_values.end(), 0);
	}
	else
		m_stage += 1 << m_shift;

	assert((m_stage & ~m_mask) == 0);
}

}