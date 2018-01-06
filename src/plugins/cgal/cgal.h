#pragma once

#include <vector>

#include <OpenEXR/ImathVec.h>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
// #include <CGAL/Polyhedron_3.h>

namespace possumwood {

typedef CGAL::Simple_cartesian<float> CGALKernel;
typedef CGAL::Surface_mesh<CGALKernel::Point_3> CGALPolyhedron;
// typedef CGAL::Polyhedron_3<CGALKernel> CGALPolyhedron;

class Meshes {
  public:
	class Item {
	  public:
		const std::string& name() const;
		const CGALPolyhedron& mesh() const;

		bool operator==(const Item& i) const;
		bool operator!=(const Item& i) const;

	  private:
		Item(const std::string& name, std::unique_ptr<CGALPolyhedron>&& mesh);

		std::string m_name;
		std::shared_ptr<const CGALPolyhedron> m_mesh;

		friend class Meshes;
	};

	void addMesh(const std::string& name, std::unique_ptr<CGALPolyhedron>&& mesh);

	typedef std::vector<Item>::const_iterator const_iterator;

	const_iterator begin() const;
	const_iterator end() const;

	std::size_t size() const;
	bool empty() const;

	bool operator==(const Meshes& m) const;
	bool operator!=(const Meshes& m) const;

  private:
	std::vector<Item> m_data;
};
}
