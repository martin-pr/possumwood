#pragma once

#include <vector>

#include "cgal.h"
#include "mesh.h"

namespace possumwood {

class Meshes {
  public:
	Mesh::MeshData& addMesh(const std::string& name);
	void addMesh(const Mesh& mesh);

	typedef std::vector<Mesh>::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;

	typedef std::vector<Mesh>::iterator iterator;
	iterator begin();
	iterator end();

	Mesh& operator[](std::size_t index);
	const Mesh& operator[](std::size_t index) const;

	std::size_t size() const;
	bool empty() const;

	bool operator==(const Meshes& m) const;
	bool operator!=(const Meshes& m) const;

  private:
	std::vector<Mesh> m_data;
};

std::ostream& operator<<(std::ostream& out, const Meshes& m);

}  // namespace possumwood
