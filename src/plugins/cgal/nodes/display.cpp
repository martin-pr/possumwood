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
		for(auto it = mesh->faces_begin(); it != mesh->faces_end(); ++it) {
			auto vertices = mesh->vertices_around_face(mesh->halfedge(*it));

			if(vertices.size() >= 2) {
				auto it1 = vertices.begin();

				auto it2 = vertices.begin();
				++it2;

				while(it1 != vertices.end()) {
					auto& p1 = mesh->point(*it1);
					glVertex3f(p1.x(), p1.y(), p1.z());

					auto& p2 = mesh->point(*it2);
					glVertex3f(p2.x(), p2.y(), p2.z());

					++it1;
					++it2;

					if(it2 == vertices.end())
						it2 = vertices.begin();
				}
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
