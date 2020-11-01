#include "cgal.h"

namespace possumwood {

unsigned long getNewFaceId() {
	static unsigned long s_id = 0;
	return s_id++;
}

std::ostream& operator<<(std::ostream& out, const CGALNefPolyhedron& nef) {
	out << "nef polyhedron - " << (*nef).number_of_vertices() << " vertices, " << (*nef).number_of_facets()
	    << " facets, " << (*nef).number_of_volumes() << " volumes";

	return out;
}

}  // namespace possumwood
