#include "skinned_vertices.h"

#include <cassert>

namespace anim {

SkinnedVertices::Vertex::Vertex(const Imath::V3f& pos, const Skinning& skin)
    : m_pos(pos), m_skinning(skin) {
}

const Imath::V3f& SkinnedVertices::Vertex::pos() const {
	return m_pos;
}

void SkinnedVertices::Vertex::setPos(const Imath::V3f& p) {
	m_pos = p;
}

const Skinning& SkinnedVertices::Vertex::skinning() const {
	return m_skinning;
}

void SkinnedVertices::Vertex::setSkinning(const Skinning& skin) {
	m_skinning = skin;
}

SkinnedVertices::Vertex& SkinnedVertices::add(const Imath::V3f& pos,
                                              const Skinning& skin) {
	m_vertices.push_back(Vertex(pos, skin));

	return m_vertices.back();
}

std::size_t SkinnedVertices::size() const {
	return m_vertices.size();
}

SkinnedVertices::Vertex& SkinnedVertices::operator[](std::size_t index) {
	return m_vertices[index];
}

const SkinnedVertices::Vertex& SkinnedVertices::operator[](std::size_t index) const {
	return m_vertices[index];
}

SkinnedVertices::const_iterator SkinnedVertices::begin() const {
	return m_vertices.begin();
}

SkinnedVertices::const_iterator SkinnedVertices::end() const {
	return m_vertices.end();
}

SkinnedVertices::iterator SkinnedVertices::begin() {
	return m_vertices.begin();
}

SkinnedVertices::iterator SkinnedVertices::end() {
	return m_vertices.end();
}

}
