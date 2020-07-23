#pragma once

#include "meshes.h"

namespace possumwood {
namespace cgal {

class PolyhedronWrapper {
  public:
	class Face {
	  public:
		void addVertex(std::size_t v);

	  private:
		std::vector<std::size_t> m_vertices;

		friend class PolyhedronWrapper;
	};

	PolyhedronWrapper(const std::string& name);

	std::size_t addPoint(float x, float y, float z);
	void addFace(const Face& f);

	operator Meshes() const;

  private:
	std::string m_name;
	std::vector<possumwood::CGALKernel::Point_3> m_points;
	std::vector<std::vector<std::size_t>> m_faces;
};

}  // namespace cgal
}  // namespace possumwood
