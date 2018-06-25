#pragma once

#include <vector>

#include "cgal.h"
#include "mesh.h"

namespace possumwood {

class Meshes {
  public:
	Mesh& addMesh(const std::string& name);

	typedef std::vector<Mesh>::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;

	typedef std::vector<Mesh>::iterator iterator;
	iterator begin();
	iterator end();

	std::size_t size() const;
	bool empty() const;

	bool operator==(const Meshes& m) const;
	bool operator!=(const Meshes& m) const;

  private:
	std::vector<Mesh> m_data;
};

std::ostream& operator << (std::ostream& out, const Meshes& m);

}
