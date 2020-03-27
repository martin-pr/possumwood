#include "t_links.h"

#include <cassert>
#include <limits>

namespace lightfields {

TLinks::TLinks(const V2i& size) : m_size(size), m_data(size.x*size.y, Link(0, std::numeric_limits<int>::max()/2)) {
}

Link& TLinks::edge(const V2i& i) {
	assert(i.x >= 0 && i.x < m_size.x);
	assert(i.y >= 0 && i.y < m_size.y);

	const std::size_t index = i.x + i.y * m_size.x;
	assert(index < m_data.size());

	return m_data[index];
}

const Link& TLinks::edge(const V2i& i) const {
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
