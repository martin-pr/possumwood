#include "skinned_mesh.h"

namespace anim {

SkinnedMesh::SkinnedMesh() {
}

const std::string& SkinnedMesh::name() const {
	return m_name;
}

void SkinnedMesh::setName(const std::string& name) {
	m_name = name;
}

SkinnedVertices& SkinnedMesh::vertices() {
	return m_vertices;
}

const SkinnedVertices& SkinnedMesh::vertices() const {
	return m_vertices;
}

std::vector<Imath::V3f>& SkinnedMesh::normals() {
	return m_normals;
}

const std::vector<Imath::V3f>& SkinnedMesh::normals() const {
	return m_normals;
}

Polygons& SkinnedMesh::polygons() {
	return m_polygons;
}

const Polygons& SkinnedMesh::polygons() const {
	return m_polygons;
}

std::size_t SkinnedMesh::boneCount() const {
	return m_vertices.boneCount();
}

}
