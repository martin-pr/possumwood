#include "generic_polymesh.inl"

#include <iostream>

namespace possumwood {
namespace polymesh {

GenericPolymesh::GenericPolymesh() : m_indices(this), m_polygons(this) {
}

GenericPolymesh::GenericPolymesh(const GenericPolymesh& p) :
	m_vertices(p.m_vertices),
	m_indices(this, p.m_indices),
	m_polygons(this, p.m_polygons),
	m_vertexIndices(p.m_vertexIndices),
	m_polygonPointers(p.m_polygonPointers) {
}

GenericPolymesh& GenericPolymesh::operator = (const GenericPolymesh& p) {
	m_vertices = p.m_vertices;
	m_indices = Indices(this, p.m_indices);
	m_polygons = Polygons(this, p.m_polygons);
	m_vertexIndices = p.m_vertexIndices;
	m_polygonPointers = p.m_polygonPointers;

	return *this;
}

bool GenericPolymesh::operator ==(const GenericPolymesh& pm) const {
	return m_vertices == pm.m_vertices &&
		m_indices == pm.m_indices &&
		m_polygons == pm.m_polygons &&
		m_vertexIndices == pm.m_vertexIndices &&
		m_polygonPointers == pm.m_polygonPointers;
}

bool GenericPolymesh::operator !=(const GenericPolymesh& pm) const {
	return m_vertices != pm.m_vertices ||
		m_indices != pm.m_indices ||
		m_polygons != pm.m_polygons ||
		m_vertexIndices != pm.m_vertexIndices ||
		m_polygonPointers != pm.m_polygonPointers;
}

const GenericPolymesh::Vertices& GenericPolymesh::vertices() const {
	return m_vertices;
}

GenericPolymesh::Vertices& GenericPolymesh::vertices() {
	return m_vertices;
}

const GenericPolymesh::Polygons& GenericPolymesh::polygons() const {
	return m_polygons;
}

GenericPolymesh::Polygons& GenericPolymesh::polygons() {
	return m_polygons;
}

const GenericPolymesh::Indices& GenericPolymesh::indices() const {
	return m_indices;
}

GenericPolymesh::Indices& GenericPolymesh::indices() {
	return m_indices;
}

///////

GenericPolymesh::Vertex::Vertex(GenericContainer<Vertex>* parent, std::size_t index) : m_parent(parent), m_index(index) {
}

std::size_t GenericPolymesh::Vertex::id() const {
	return m_index;
}

GenericPolymesh::Vertex& GenericPolymesh::Vertex::operator += (long d) {
	m_index += d;

	return *this;
}

void GenericPolymesh::Vertex::operator++() {
	++m_index;
}

///////

GenericPolymesh::Vertices::iterator GenericPolymesh::Vertices::add() {
	return begin() + GenericContainer<Vertex>::add();
}

///////

GenericPolymesh::Index::Index(GenericContainer<Index>* parent, std::size_t index) : m_parent(parent), m_index(index) {
	assert(dynamic_cast<Indices*>(parent) != nullptr);
	Indices* ii = (Indices*)parent;

	m_vertex = std::unique_ptr<Vertex>(new Vertex(&ii->m_parent->m_vertices, ii->m_parent->m_vertexIndices[m_index]));
}

GenericPolymesh::Index::Index(const Index& i) : m_parent(i.m_parent), m_index(i.m_index) {
	m_vertex = std::unique_ptr<Vertex>(new Vertex(*i.m_vertex));
}

GenericPolymesh::Index& GenericPolymesh::Index::operator=(const Index& i) {
	m_parent = i.m_parent;
	m_index = i.m_index;
	m_vertex = std::unique_ptr<Vertex>(new Vertex(*i.m_vertex));

	return *this;
}

GenericPolymesh::Index& GenericPolymesh::Index::operator += (long d) {
	m_index += d;

	assert(dynamic_cast<Indices*>(m_parent) != nullptr);
	Indices* ii = (Indices*)m_parent;
	m_vertex = std::unique_ptr<Vertex>(new Vertex(&ii->m_parent->m_vertices, ii->m_parent->m_vertexIndices[m_index]));

	return *this;
}

void GenericPolymesh::Index::operator++() {
	++m_index;

	assert(dynamic_cast<Polygons*>(m_parent) != nullptr);
	Polygons* polys = (Polygons*)m_parent;
	m_vertex = std::unique_ptr<Vertex>(new Vertex(&polys->m_parent->m_vertices, polys->m_parent->m_vertexIndices[m_index]));
}

GenericPolymesh::Vertex& GenericPolymesh::Index::vertex() {
	assert(m_vertex);
	return *m_vertex;
}

const GenericPolymesh::Vertex& GenericPolymesh::Index::vertex() const {
	assert(m_vertex);
	return *m_vertex;
}

GenericPolymesh::Indices::Indices(GenericPolymesh* parent, const GenericContainer<Index>& value) : GenericContainer<Index>(value), m_parent(parent) {
}

///////

GenericPolymesh::Polygon::Polygon(GenericContainer<Polygon>* parent, std::size_t index) : m_index(index) {
	assert(dynamic_cast<Polygons*>(parent) != nullptr);
	Polygons* polys = dynamic_cast<Polygons*>(parent);

	m_parent = polys->m_parent;
}

GenericPolymesh::Polygon& GenericPolymesh::Polygon::operator += (long d) {
	m_index += d;
	return *this;
}

void GenericPolymesh::Polygon::operator++() {
	++m_index;
}

std::size_t GenericPolymesh::Polygon::size() const {
	if(m_index < m_parent->m_polygonPointers.size()-1)
		return m_parent->m_polygonPointers[m_index+1] - m_parent->m_polygonPointers[m_index];

	// last polygon
	assert(m_index == m_parent->m_polygonPointers.size()-1);
	return m_parent->m_indices.size() - m_parent->m_polygonPointers.back();
}

GenericPolymesh::Polygon::iterator GenericPolymesh::Polygon::begin() {
	return m_parent->m_indices.begin() + m_parent->m_polygonPointers[m_index];
}

GenericPolymesh::Polygon::iterator GenericPolymesh::Polygon::end() {
	return m_parent->m_indices.begin() + m_parent->m_polygonPointers[m_index+1];
}

GenericPolymesh::Polygon::const_iterator GenericPolymesh::Polygon::begin() const {
	const GenericContainer<Index>& indices = m_parent->m_indices;
	return indices.begin() + m_parent->m_polygonPointers[m_index];
}

GenericPolymesh::Polygon::const_iterator GenericPolymesh::Polygon::end() const {
	const GenericContainer<Index>& indices = m_parent->m_indices;
	return indices.begin() + m_parent->m_polygonPointers[m_index+1];
}

///////

GenericPolymesh::Polygons::Polygons(GenericPolymesh* parent, const GenericContainer<Polygon>& value) : GenericContainer<Polygon>(value), m_parent(parent) {

}

///////

std::ostream& operator << (std::ostream& out, const GenericPolymesh& pm) {
	out << "GenericPolymesh with " << pm.vertices().size() << " vertices";

	return out;
}

}
}
