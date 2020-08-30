#pragma once

#include <CGAL/Polyhedron_3.h>

namespace possumwood {

template <class HDS, class POINTS, class FACES>
class CGALBuilder : public CGAL::Modifier_base<HDS> {
  public:
	CGALBuilder(const POINTS& pts, const FACES& f) : m_points(&pts), m_faces(&f) {
	}
	void operator()(HDS& hds) {
		// Postcondition: hds is a valid polyhedral surface.
		CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);

		B.begin_surface(m_points->size(), m_faces->size());

		for(auto& v : *m_points)
			B.add_vertex(typename HDS::Traits::Point_3(v[0], v[1], v[2]));

		for(auto& f : *m_faces) {
			B.begin_facet();

			for(auto& i : f)
				B.add_vertex_to_facet(i);

			B.end_facet();
		}

		B.end_surface();
	}

  private:
	const POINTS* m_points;
	const FACES* m_faces;
};

}  // namespace possumwood
