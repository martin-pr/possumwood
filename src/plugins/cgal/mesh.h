#pragma once

#include "cgal.h"
#include "properties.h"

namespace possumwood {

class Mesh {
  public:
	const std::string& name() const;
	void setName(const std::string& name);

	const CGALPolyhedron& polyhedron() const;
	CGALPolyhedron& polyhedron();

	const Properties& vertexProperties() const;
	Properties& vertexProperties();

	const Properties& faceProperties() const;
	Properties& faceProperties();

	const Properties& halfedgeProperties() const;
	Properties& halfedgeProperties();

	bool operator==(const Mesh& i) const;
	bool operator!=(const Mesh& i) const;

  private:
	Mesh(const std::string& name);

	std::string m_name;
	std::shared_ptr<CGALPolyhedron> m_polyhedron;
	Properties m_vertexProperties, m_faceProperties, m_halfedgeProperties;

	friend class Meshes;
};
}  // namespace possumwood
