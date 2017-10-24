#include <possumwood_sdk/node_implementation.h>

#include <GL/gl.h>

#include "datatypes/polyhedron.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const possumwood::CGALPolyhedron>> a_mesh;

dependency_graph::State draw(const dependency_graph::Values& data) {
	const std::shared_ptr<const possumwood::CGALPolyhedron> mesh = data.get(a_mesh);

	if(mesh) {
		glColor3f(1, 0, 0);

		glDisable(GL_LIGHTING);

		glBegin(GL_LINES);
		for(auto it = mesh->facets_begin(); it != mesh->facets_end(); ++it) {
			auto heIt = it->facet_begin();
			for(unsigned a = 0; a < it->size(); ++a) {
				auto& p1 = heIt->vertex()->point();
				glVertex3f(p1.x(), p1.y(), p1.z());

				++heIt;

				auto& p2 = heIt->vertex()->point();
				glVertex3f(p2.x(), p2.y(), p2.z());
			}
		}
		glEnd();

		glEnable(GL_LIGHTING);
	}

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_mesh, "mesh");

	meta.setDrawable(&draw);
}

possumwood::NodeImplementation s_impl("cgal/display", init);
}
