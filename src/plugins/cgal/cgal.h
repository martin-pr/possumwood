#pragma once

#include <vector>

#include <OpenEXR/ImathVec.h>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>

namespace possumwood {

typedef CGAL::Simple_cartesian<float> CGALKernel;
typedef CGAL::Surface_mesh<CGALKernel::Point_3> CGALPolyhedron;

class Meshes {
  public:
	class Item {
	  public:
		const std::string& name() const {
			return m_name;
		}

		const CGALPolyhedron& mesh() const {
			assert(m_mesh != nullptr);
			return *m_mesh;
		}

		bool operator==(const Item& i) const {
			return m_name == i.m_name && m_mesh == i.m_mesh;
		}

		bool operator!=(const Item& i) const {
			return m_name != i.m_name || m_mesh != i.m_mesh;
		}

	  private:
		Item(const std::string& name, std::unique_ptr<CGALPolyhedron>&& mesh) : m_name(name), m_mesh(mesh.release()) {
		}

		std::string m_name;
		std::shared_ptr<const CGALPolyhedron> m_mesh;

		friend class Meshes;
	};

	void addMesh(const std::string& name, std::unique_ptr<CGALPolyhedron>&& mesh) {
		assert(mesh != nullptr);

		m_data.push_back(Item(name, std::move(mesh)));
	}

	typedef std::vector<Item>::const_iterator const_iterator;

	const_iterator begin() const {
		return m_data.begin();
	}

	const_iterator end() const {
		return m_data.end();
	}

	std::size_t size() const {
		return m_data.size();
	}

	bool empty() const {
		return m_data.empty();
	}

	bool operator==(const Meshes& m) const {
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

	bool operator!=(const Meshes& m) const {
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

  private:
	std::vector<Item> m_data;
};
}
