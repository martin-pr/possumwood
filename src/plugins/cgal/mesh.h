#pragma once

#include "cgal.h"
#include "properties.h"

namespace possumwood {

/// A COW mesh data structure - calling edit() allows for mesh editing, but also makes sure that
/// any underlying data are deduplicated. Otherwise, the copying of Mesh instances produces read-only
/// shallow copies.
/// Currently, there is no protection for mis-use of the COW - storing a reference to the MeshData
/// datastructure is a terrible idea.
class Mesh {
  public:
	class MeshData {
	  public:
		CGALPolyhedron& polyhedron();
		Properties& vertexProperties();
		Properties& faceProperties();
		Properties& halfedgeProperties();

	  private:
		MeshData() = default;
		MeshData(const MeshData&) = default;
		MeshData& operator=(const MeshData&) = default;

		CGALPolyhedron m_polyhedron;
		Properties m_vertexProperties, m_faceProperties, m_halfedgeProperties;

		friend class Mesh;
	};

	/// Returns a non-const reference to the underlying data, allowing them to be edited.
	/// If this Mesh instance doesn't have a unique data instance, the underlying data will be duplicated wholesale.
	MeshData& edit();

	const std::string& name() const;
	void setName(const std::string& name);

	const CGALPolyhedron& polyhedron() const;
	const Properties& vertexProperties() const;
	const Properties& faceProperties() const;
	const Properties& halfedgeProperties() const;

	bool operator==(const Mesh& i) const;
	bool operator!=(const Mesh& i) const;

  private:
	Mesh(const std::string& name);

	std::string m_name;

	std::shared_ptr<MeshData> m_data;

	friend class Meshes;
};

}  // namespace possumwood
