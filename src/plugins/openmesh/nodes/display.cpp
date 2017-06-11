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

		glBegin(GL_POINTS);
		for(auto it = mesh->vertices_begin(); it != mesh->vertices_end(); ++it) {
			const auto& pt = mesh->point(*it);
			glVertex3f(pt[0], pt[1], pt[2]);
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
