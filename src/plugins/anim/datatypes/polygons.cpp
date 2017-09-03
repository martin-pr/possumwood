#include "polygons.h"

#include <cassert>

namespace anim {

std::array<std::size_t, 3>& Polygons::add(std::size_t p1, std::size_t p2, std::size_t p3) {
	m_polygons.push_back(std::array<std::size_t, 3>{p1, p2, p3});
	return m_polygons.back();
}

std::array<std::size_t, 3>& Polygons::operator[](std::size_t index) {
	assert(index < m_polygons.size());
	return m_polygons[index];
}

const std::array<std::size_t, 3>& Polygons::operator[](std::size_t index) const {
	assert(index < m_polygons.size());
	return m_polygons[index];
}

bool Polygons::empty() const {
	return m_polygons.empty();
}

std::size_t Polygons::size() const {
	return m_polygons.size();
}

Polygons::const_iterator Polygons::begin() const {
	return m_polygons.begin();
}

Polygons::const_iterator Polygons::end() const {
	return m_polygons.end();
}

Polygons::iterator Polygons::begin() {
	return m_polygons.begin();
}

Polygons::iterator Polygons::end() {
	return m_polygons.end();
}

}
