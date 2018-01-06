#pragma once

#include <vector>

#include "cgal.h"
#include "mesh.h"

namespace possumwood {

class Meshes {
  public:
	void addMesh(const std::string& name, std::unique_ptr<CGALPolyhedron>&& mesh);

	typedef std::vector<Mesh>::const_iterator const_iterator;

	const_iterator begin() const;
	const_iterator end() const;

	std::size_t size() const;
	bool empty() const;

	bool operator==(const Meshes& m) const;
	bool operator!=(const Meshes& m) const;

  private:
	std::vector<Mesh> m_data;
};
}
