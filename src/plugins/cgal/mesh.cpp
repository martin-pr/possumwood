#include "mesh.h"

namespace possumwood {

Mesh::Mesh(const std::string& name) : m_name(name), m_polyhedron(new CGALPolyhedron()) {
}

const std::string& Mesh::name() const {
	return m_name;
}

void Mesh::setName(const std::string& name) {
	m_name = name;
}

const CGALPolyhedron& Mesh::polyhedron() const {
	return *m_polyhedron;
}

CGALPolyhedron& Mesh::polyhedron() {
	// make sure this polyhedron is unique before changing it
	if(m_polyhedron.use_count() > 1)
		m_polyhedron = std::shared_ptr<CGALPolyhedron>(new CGALPolyhedron(*m_polyhedron));

	return *m_polyhedron;
}

const Properties& Mesh::vertexProperties() const {
	return m_vertexProperties;
}

Properties& Mesh::vertexProperties() {
	return m_vertexProperties;
}

const Properties& Mesh::faceProperties() const {
	return m_faceProperties;
}

Properties& Mesh::faceProperties() {
	return m_faceProperties;
}

const Properties& Mesh::halfedgeProperties() const {
	return m_halfedgeProperties;
}

Properties& Mesh::halfedgeProperties() {
	return m_halfedgeProperties;
}

bool Mesh::operator==(const Mesh& i) const {
	return m_name == i.m_name && m_polyhedron == i.m_polyhedron && m_faceProperties == i.m_faceProperties &&
	       m_vertexProperties == i.m_vertexProperties && m_halfedgeProperties == i.m_halfedgeProperties;
}

bool Mesh::operator!=(const Mesh& i) const {
	return m_name != i.m_name || m_polyhedron != i.m_polyhedron || m_faceProperties != i.m_faceProperties ||
	       m_vertexProperties != i.m_vertexProperties || m_halfedgeProperties != i.m_halfedgeProperties;
}
}  // namespace possumwood
