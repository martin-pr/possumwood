#pragma once

#include <CGAL/Simple_cartesian.h>
#include <OpenEXR/ImathVec.h>
// #include <CGAL/Surface_mesh.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Polyhedron_3.h>

#include "property.inl"

namespace possumwood {

template <class Refs>
struct CGALFace : public CGAL::HalfedgeDS_face_base<Refs> {
  public:
	PropertyKey& property_key() {
		return m_propKey;
	}

	const PropertyKey& property_key() const {
		return m_propKey;
	}

  private:
	PropertyKey m_propKey;
};

template <class Refs>
class CGALVertex : public CGAL::HalfedgeDS_vertex_base<Refs> {
  public:
	typedef CGAL::Simple_cartesian<float>::Point_3 Point;

	CGALVertex(const Point& p = Point(0, 0, 0)) : m_point(p) {
	}

	PropertyKey& property_key() {
		return m_propKey;
	}

	Point& point() {
		return m_point;
	}

	const PropertyKey& property_key() const {
		return m_propKey;
	}

	const Point& point() const {
		return m_point;
	}

  private:
	Point m_point;
	PropertyKey m_propKey;
};

template <class Refs>
struct CGALHalfedge : public CGAL::HalfedgeDS_halfedge_base<Refs> {
  public:
	PropertyKey& property_key() {
		return m_propKey;
	}

	const PropertyKey& property_key() const {
		return m_propKey;
	}

  private:
	PropertyKey m_propKey;
};

struct CGALPolyhedronItems : public CGAL::Polyhedron_items_3 {
	template <class Refs, class Traits>
	struct Face_wrapper {
		typedef CGALFace<Refs> Face;
	};

	template <class Refs, class Traits>
	struct Vertex_wrapper {
		typedef CGALVertex<Refs> Vertex;
	};

	template <class Refs, class Traits>
	struct Halfedge_wrapper {
		typedef CGALHalfedge<Refs> Halfedge;
	};
};

typedef CGAL::Simple_cartesian<float> CGALKernel;
// typedef CGAL::Surface_mesh<CGALKernel::Point_3> CGALPolyhedron;
typedef CGAL::Polyhedron_3<CGALKernel, CGALPolyhedronItems> CGALPolyhedron;

typedef CGAL::Exact_predicates_exact_constructions_kernel NefKernel;
typedef CGAL::Nef_polyhedron_3<NefKernel> CGALNef;

class CGALNefPolyhedron {
  public:
	CGALNefPolyhedron() : m_data(new CGALNef()) {
	}

	CGALNefPolyhedron(std::unique_ptr<CGALNef> nef) : m_data(nef.release()) {
	}

	const CGALNef& operator*() const {
		return *m_data;
	}

	operator const CGALNef&() const {
		return *m_data;
	}

  private:
	std::shared_ptr<const CGALNef> m_data;
};

std::ostream& operator<<(std::ostream& out, const CGALNefPolyhedron& nef);

}  // namespace possumwood
