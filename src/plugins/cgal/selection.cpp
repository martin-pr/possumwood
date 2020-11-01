#include "selection.h"

namespace possumwood {

FaceSelection::Item::Item(const Mesh& mesh) : m_mesh(mesh) {
}

FaceSelection::Item::const_iterator FaceSelection::Item::begin() const {
	return m_faces.begin();
}

FaceSelection::Item::const_iterator FaceSelection::Item::end() const {
	return m_faces.end();
}

FaceSelection::Item::iterator FaceSelection::Item::begin() {
	return m_faces.begin();
}

FaceSelection::Item::iterator FaceSelection::Item::end() {
	return m_faces.end();
}

void FaceSelection::Item::push_back(const possumwood::CGALPolyhedron::Facet_const_handle& value) {
	m_faces.insert(value);
}

bool FaceSelection::Item::contains(const possumwood::CGALPolyhedron::Facet_const_handle& value) const {
	return m_faces.find(value) != m_faces.end();
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
	for(auto& i : sel) {
		out << "  " << i.size() << " items with IDs:";
		for(auto& num : i)
			out << " " << num->halfedge()->property_key();
		out << std::endl;
	}
	return out;
}

}  // namespace possumwood
