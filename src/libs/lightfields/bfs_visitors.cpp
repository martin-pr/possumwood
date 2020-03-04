#include "bfs_visitors.h"

#include <limits>
#include <cassert>

namespace lightfields {

std::size_t BFSVisitors::vec2index(const V2i& v) const {
	if(v.x == -1 && v.y == -1)
		return std::numeric_limits<std::size_t>::max();

	assert(v.x >= 0 && v.x < m_size.x);
	assert(v.y >= 0 && v.y < m_size.y);

	const std::size_t index = v.x + v.y * m_size.x;
	assert(index < m_values.size());

	return index;
}

V2i BFSVisitors::index2vec(std::size_t i) const {
	assert(i < m_values.size());

	return V2i(i % m_size.x, i / m_size.x);
}

BFSVisitors::BFSVisitors(const V2i& size) : m_size(size), m_stage(0), m_mask(std::numeric_limits<std::size_t>::max()), m_shift(0), m_values(m_size.x*m_size.y) {
	std::size_t count = m_size.x * m_size.y;
	while(count > 0) {
		count = count >> 1;
		m_mask = m_mask << 1;
		++m_shift;
	}

	m_stage = 1 << m_shift;

	assert(m_mask == (std::numeric_limits<std::size_t>::max() << m_shift));
}

bool BFSVisitors::visited(const V2i& v) const {
	const std::size_t index = vec2index(v);

	assert(index < m_values.size());
	return (m_values[index] & m_mask) == m_stage;
}

V2i BFSVisitors::parent(const V2i& v) const {
	const std::size_t index = vec2index(v);

	assert(visited(v));

	if((m_values[index] | m_mask) == std::numeric_limits<std::size_t>::max())
		return V2i(-1, -1);

	return index2vec(m_values[index] & ~m_mask);
}

void BFSVisitors::visit(const V2i& index, const V2i& parent) {
	assert(!visited(index));

	const std::size_t parent_id = vec2index(parent);
	assert(parent_id == std::numeric_limits<std::size_t>::max() || parent_id < m_values.size());

	m_values[vec2index(index)] = (parent_id & ~m_mask) | m_stage;
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
