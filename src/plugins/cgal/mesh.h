#pragma once

#include "cgal.h"

namespace possumwood {

class Mesh {
  public:
	const std::string& name() const;
	const CGALPolyhedron& polyhedron() const;

	bool operator==(const Mesh& i) const;
	bool operator!=(const Mesh& i) const;

  private:
	Mesh(const std::string& name, std::unique_ptr<CGALPolyhedron>&& mesh);

	std::string m_name;
	std::shared_ptr<const CGALPolyhedron> m_mesh;

	friend class Meshes;
};
}
