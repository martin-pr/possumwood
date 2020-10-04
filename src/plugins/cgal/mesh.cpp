#include "mesh.h"

namespace possumwood {

CGALPolyhedron& Mesh::MeshData::polyhedron() {
	return m_polyhedron;
}

Properties& Mesh::MeshData::vertexProperties() {
	return m_vertexProperties;
}

Properties& Mesh::MeshData::faceProperties() {
	return m_faceProperties;
}

Properties& Mesh::MeshData::halfedgeProperties() {
	return m_halfedgeProperties;
}

//////////////////////

Mesh::Mesh(const std::string& name) : m_name(name), m_data(new MeshData()) {
}

Mesh::MeshData& Mesh::edit() {
	// make sure this polyhedron is unique before changing it
	if(m_data.use_count() > 1)
		m_data = std::shared_ptr<MeshData>(new MeshData(*m_data));

	return *m_data;
}

const std::string& Mesh::name() const {
	return m_name;
}

void Mesh::setName(const std::string& name) {
	m_name = name;
}

const CGALPolyhedron& Mesh::polyhedron() const {
	return m_data->polyhedron();
}

const Properties& Mesh::vertexProperties() const {
	return m_data->vertexProperties();
}

const Properties& Mesh::faceProperties() const {
	return m_data->faceProperties();
}

const Properties& Mesh::halfedgeProperties() const {
	return m_data->halfedgeProperties();
}

bool Mesh::operator==(const Mesh& i) const {
	return m_name == i.m_name && m_data == i.m_data;
}

bool Mesh::operator!=(const Mesh& i) const {
	return m_name != i.m_name || m_data != i.m_data;
}

}  // namespace possumwood
