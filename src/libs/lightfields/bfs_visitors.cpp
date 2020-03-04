#include "bfs_visitors.h"

#include <limits>
#include <cassert>

namespace lightfields {

BFSVisitors::BFSVisitors(std::size_t count) : m_stage(0), m_mask(std::numeric_limits<std::size_t>::max()), m_shift(0), m_values(count) {
	while(count > 0) {
		count = count >> 1;
		m_mask = m_mask << 1;
		++m_shift;
	}

	m_stage = 1 << m_shift;

	assert(m_mask == (std::numeric_limits<std::size_t>::max() << m_shift));
}

bool BFSVisitors::visited(std::size_t index) const {
	assert(index < m_values.size());
	return (m_values[index] & m_mask) == m_stage;
}

std::size_t BFSVisitors::parent(std::size_t index) const {
	assert(visited(index));

	if((m_values[index] | m_mask) == std::numeric_limits<std::size_t>::max())
		return std::numeric_limits<std::size_t>::max();

	return m_values[index] & ~m_mask;
}

void BFSVisitors::visit(std::size_t index, std::size_t parent) {
	assert(!visited(index));
	assert(parent == std::numeric_limits<std::size_t>::max() || parent < m_values.size());

	m_values[index] = (parent & ~m_mask) | m_stage;
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
