#include <possumwood/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include <GL/glut.h>

#include "io/mesh.h"
#include "openmesh.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const Mesh>> a_mesh;

void draw(const dependency_graph::Values& data) {
	const std::shared_ptr<const Mesh> mesh = data.get(a_mesh);

	if(mesh) {
		glColor3f(1, 0, 0);

		// glBegin(GL_POINTS);
		// for(auto it = mesh->vertices_begin(); it != mesh->vertices_end(); ++it) {
		// 	const auto& pt = mesh->point(*it);
		// 	glVertex3f(pt[0], pt[1], pt[2]);
		// }
		// glEnd();

		glBegin(GL_LINES);
		for(auto it = mesh->edges_begin(); it != mesh->edges_end(); ++it) {
			const auto he0h = mesh->halfedge_handle(*it, 0);

			const auto v1 = mesh->from_vertex_handle(he0h);
			const auto v2 = mesh->to_vertex_handle(he0h);

			const auto& pt1 = mesh->point(v1);
			const auto& pt2 = mesh->point(v2);

			glVertex3f(pt1[0], pt1[1], pt1[2]);
			glVertex3f(pt2[0], pt2[1], pt2[2]);
		}
		glEnd();
	}
}

void init(Metadata& meta) {
	meta.addAttribute(a_mesh, "mesh");

	meta.setDraw(draw);
}

NodeImplementation s_impl("openmesh/display", init);

}
