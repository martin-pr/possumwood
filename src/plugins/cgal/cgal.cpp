#include "cgal.h"

namespace possumwood {

Meshes::Item::Item(const std::string& name, std::unique_ptr<CGALPolyhedron>&& mesh)
    : m_name(name), m_mesh(mesh.release()) {
}

const std::string& Meshes::Item::name() const {
	return m_name;
}

const CGALPolyhedron& Meshes::Item::mesh() const {
	assert(m_mesh != nullptr);
	return *m_mesh;
}

bool Meshes::Item::operator==(const Item& i) const {
	return m_name == i.m_name && m_mesh == i.m_mesh;
}

bool Meshes::Item::operator!=(const Item& i) const {
	return m_name != i.m_name || m_mesh != i.m_mesh;
}

void Meshes::addMesh(const std::string& name, std::unique_ptr<CGALPolyhedron>&& mesh) {
	assert(mesh != nullptr);

	m_data.push_back(Item(name, std::move(mesh)));
}

Meshes::const_iterator Meshes::begin() const {
	return m_data.begin();
}

Meshes::const_iterator Meshes::end() const {
	return m_data.end();
}

std::size_t Meshes::size() const {
	return m_data.size();
}

bool Meshes::empty() const {
	return m_data.empty();
}

bool Meshes::operator==(const Meshes& m) const {
	if(size() != m.size())
		return false;

	auto it1 = begin();
	auto it2 = m.begin();

	while(it1 != end()) {
		if(*it1 != *it2)
			return false;

		++it1;
		++it2;
	}

	return true;
}

bool Meshes::operator!=(const Meshes& m) const {
	if(size() != m.size())
		return true;

	auto it1 = begin();
	auto it2 = m.begin();

	while(it1 != end()) {
		if(*it1 != *it2)
			return true;

		++it1;
		++it2;
	}

	return false;
}
}
