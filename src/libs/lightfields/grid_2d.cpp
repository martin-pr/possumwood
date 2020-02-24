#include "grid_2d.h"

#include <cassert>

namespace lightfields {

Grid2D::Grid2D(const Imath::V2i& size) : m_size(size), m_data(size[0]*size[1]) {
}

Grid2D::Edge& Grid2D::edge(const Imath::V2i& i) {
	assert(i[0] >= 0 && i[0] < m_size[0]);
	assert(i[1] >= 0 && i[1] < m_size[1]);

	const std::size_t index = i[0] + i[1] * m_size[0];
	assert(index < m_data.size());

	assert(m_data[index].forward >= 0.0f);
	assert(m_data[index].backward >= 0.0f);

	return m_data[index];
}

const Grid2D::Edge& Grid2D::edge(const Imath::V2i& i) const {
	assert(i[0] >= 0 && i[0] < m_size[0]);
	assert(i[1] >= 0 && i[1] < m_size[1]);

	const std::size_t index = i[0] + i[1] * m_size[0];
	assert(index < m_data.size());

	assert(m_data[index].forward >= 0.0f);
	assert(m_data[index].backward >= 0.0f);

	return m_data[index];
}

const Imath::V2i& Grid2D::size() const {
	return m_size;
}

}
