#include "n_links.h"

#include <cassert>
#include <stdexcept>

namespace lightfields {

NLinks::NLinks(const V2i& size, int n_link_value)
    : m_size(size), m_horiz((size.x - 1) * size.y, Link(n_link_value)),
      m_vert(size.x * (size.y - 1), Link(n_link_value)) {
}

std::size_t NLinks::h_index(const V2i& i) const {
	return i.x + i.y * (m_size.x - 1);
}

std::size_t NLinks::v_index(const V2i& i) const {
	return i.x + i.y * m_size.x;
}

Link::Direction& NLinks::edge(const V2i& src, const V2i& dest) {
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

const Link::Direction& NLinks::edge(const V2i& src, const V2i& dest) const {
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

V2i NLinks::size() const {
	return m_size;
}

}  // namespace lightfields
