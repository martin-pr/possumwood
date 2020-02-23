#include "graph_2d.h"

#include <cassert>

namespace lightfields {

Graph2D::Graph2D(const Imath::V2i& size, float n_link_value) : m_size(size), m_horiz((size[0]-1) * size[1], n_link_value), m_vert(size[0] * (size[1] - 1), n_link_value) {
}

std::size_t Graph2D::h_index(const Imath::V2i& i) const {
	return i[0] + i[1] * (m_size[0] - 1);
}

std::size_t Graph2D::v_index(const Imath::V2i& i) const {
	return i[0] + i[1] * m_size[0];
}

float& Graph2D::edge(const Imath::V2i& src, const Imath::V2i& dest) {
	assert((src[0] - dest[0]) * (src[0] - dest[0]) + (src[1] - dest[1]) * (src[1] - dest[1]) == 1);

	assert(src[0] >= 0 && src[0] < m_size[0]);
	assert(dest[0] >= 0 && dest[0] < m_size[0]);
	assert(src[1] >= 0 && src[1] < m_size[1]);
	assert(src[1] >= 0 && dest[1] < m_size[1]);

	if(src[0] < dest[0])
		return m_horiz[h_index(src)].forward;

	if(src[0] > dest[0])
		return m_horiz[h_index(dest)].backward;

	if(src[1] < dest[1])
		return m_vert[v_index(src)].forward;

	if(src[1] > dest[1])
		return m_vert[v_index(dest)].backward;

	assert(false && "bad edge index");

	// avoid GCC warning
	static float s_return_value_hack;
	return s_return_value_hack;
}

const float& Graph2D::edge(const Imath::V2i& src, const Imath::V2i& dest) const {
	assert((src[0] - dest[0]) * (src[0] - dest[0]) + (src[1] - dest[1]) * (src[1] - dest[1]) == 1);

	assert(src[0] >= 0 && src[0] < m_size[0]);
	assert(dest[0] >= 0 && dest[0] < m_size[0]);
	assert(src[1] >= 0 && src[1] < m_size[1]);
	assert(src[1] >= 0 && dest[1] < m_size[1]);

	if(src[0] < dest[0])
		return m_horiz[h_index(src)].forward;

	if(src[0] > dest[0])
		return m_horiz[h_index(dest)].backward;

	if(src[1] < dest[1])
		return m_vert[v_index(src)].forward;

	if(src[1] > dest[1])
		return m_vert[v_index(dest)].backward;

	assert(false && "bad edge index");

	// avoid GCC warning
	static float s_return_value_hack;
	return s_return_value_hack;
}

}
