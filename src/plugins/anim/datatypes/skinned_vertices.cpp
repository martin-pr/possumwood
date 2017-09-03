#include "skinned_vertices.h"

namespace anim {

SkinnedVertices::SkinnedVertices() {
}

SkinnedVertices::Vertex::Vertex(const Imath::V3f& pos,
                                const std::initializer_list<std::pair<std::size_t, float>>& weights)
    : m_pos(pos), m_weights(weights) {
}

const Imath::V3f& SkinnedVertices::Vertex::pos() const {
	return m_pos;
}

void SkinnedVertices::Vertex::setPos(const Imath::V3f& p) {
	m_pos = p;
}

void SkinnedVertices::Vertex::addWeight(const std::size_t& bone, const float& w) {
	m_weights.push_back(std::make_pair(bone, w));
}

std::size_t SkinnedVertices::Vertex::size() const {
	return m_weights.size();
}

std::pair<std::size_t, float>& SkinnedVertices::Vertex::operator[](std::size_t weightIndex) {
	return m_weights[weightIndex];
}

const std::pair<std::size_t, float>& SkinnedVertices::Vertex::operator[](std::size_t weightIndex) const {
	return m_weights[weightIndex];
}

SkinnedVertices::Vertex::const_iterator SkinnedVertices::Vertex::begin() const {
	return m_weights.begin();
}

SkinnedVertices::Vertex::const_iterator SkinnedVertices::Vertex::end() const {
	return m_weights.end();
}

SkinnedVertices::Vertex::iterator SkinnedVertices::Vertex::begin() {
	return m_weights.begin();
}

SkinnedVertices::Vertex::iterator SkinnedVertices::Vertex::end() {
	return m_weights.end();
}

SkinnedVertices::Vertex& SkinnedVertices::add(const Imath::V3f& pos,
                                              const std::initializer_list<std::pair<std::size_t, float>>& weights) {
	m_vertices.push_back(Vertex(pos, weights));
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
