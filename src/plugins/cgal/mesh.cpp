#include "mesh.h"

namespace possumwood {

Mesh::Mesh(const std::string& name, std::unique_ptr<CGALPolyhedron>&& mesh)
    : m_name(name), m_mesh(mesh.release()) {
}

const std::string& Mesh::name() const {
	return m_name;
}

const CGALPolyhedron& Mesh::mesh() const {
	assert(m_mesh != nullptr);
	return *m_mesh;
}

bool Mesh::operator==(const Mesh& i) const {
	return m_name == i.m_name && m_mesh == i.m_mesh;
}

bool Mesh::operator!=(const Mesh& i) const {
	return m_name != i.m_name || m_mesh != i.m_mesh;
}
}
