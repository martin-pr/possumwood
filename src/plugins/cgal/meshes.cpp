#include "meshes.h"

namespace possumwood {

Mesh::MeshData& Meshes::addMesh(const std::string& name) {
	m_data.push_back(Mesh(name));
	return m_data.back().edit();
}

void Meshes::addMesh(const Mesh& mesh) {
	m_data.push_back(mesh);
}

Meshes::const_iterator Meshes::begin() const {
	return m_data.begin();
}

Meshes::const_iterator Meshes::end() const {
	return m_data.end();
}

Meshes::iterator Meshes::begin() {
	return m_data.begin();
}

Meshes::iterator Meshes::end() {
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

std::ostream& operator<<(std::ostream& out, const Meshes& m) {
	out << "( " << m.size() << " meshes"
	    << ")";
	return out;
}

}  // namespace possumwood
