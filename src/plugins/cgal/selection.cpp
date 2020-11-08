#include "selection.h"

namespace possumwood {

FaceSelection::Item::Item(const Mesh& mesh) : m_mesh(mesh) {
}

void FaceSelection::Item::push_back(const possumwood::CGALPolyhedron::Facet_const_handle& value) {
	m_faces.insert(value->uniqueId());
}

void FaceSelection::Item::remove(const possumwood::CGALPolyhedron::Facet_const_handle& value) {
	auto it = m_faces.find(value->uniqueId());
	if(it != m_faces.end())
		m_faces.erase(it);
}

bool FaceSelection::Item::contains(const possumwood::CGALPolyhedron::Facet_const_handle& value) const {
	return m_faces.find(value->uniqueId()) != m_faces.end();
}

bool FaceSelection::Item::empty() const {
	return m_faces.empty();
}

std::size_t FaceSelection::Item::size() const {
	return m_faces.size();
}

const Mesh& FaceSelection::Item::mesh() const {
	return m_mesh;
}

void FaceSelection::Item::clear() {
	m_faces.clear();
}

void FaceSelection::Item::setMesh(const Mesh& mesh) {
	std::set<unsigned long> ids;
	for(auto fi = mesh.polyhedron().facets_begin(); fi != mesh.polyhedron().facets_end(); ++fi)
		ids.insert(fi->uniqueId());

	auto it = m_faces.begin();
	while(it != m_faces.end()) {
		if(ids.find(*it) == ids.end())
			m_faces.erase(it++);
		else
			++it;
	}

	m_mesh = mesh;
}

bool FaceSelection::empty() const {
	return m_items.empty();
}

std::size_t FaceSelection::size() const {
	return m_items.size();
}

FaceSelection::Item& FaceSelection::operator[](std::size_t index) {
	return m_items[index];
}

const FaceSelection::Item& FaceSelection::operator[](std::size_t index) const {
	return m_items[index];
}

void FaceSelection::push_back(const Item& item) {
	m_items.push_back(item);
}

FaceSelection::const_iterator FaceSelection::begin() const {
	return m_items.begin();
}

FaceSelection::const_iterator FaceSelection::end() const {
	return m_items.end();
}

std::ostream& operator<<(std::ostream& out, const FaceSelection& sel) {
	out << "Selection in " << sel.size() << " meshes:" << std::endl;
	for(auto& i : sel)
		out << "  " << i.mesh().name() << ": " << i.size() << "/" << i.mesh().polyhedron().size_of_facets()
		    << std::endl;
	return out;
}

}  // namespace possumwood
