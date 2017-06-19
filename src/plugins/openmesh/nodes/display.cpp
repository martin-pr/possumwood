#include <possumwood_sdk/node_implementation.h>

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
		// glColor3f(1, 0, 0);

		// glDisable(GL_LIGHTING);

		// glBegin(GL_LINES);
		// for(auto it = mesh->edges_begin(); it != mesh->edges_end(); ++it) {
		// 	const auto he0h = mesh->halfedge_handle(*it, 0);

		// 	const auto v1 = mesh->from_vertex_handle(he0h);
		// 	const auto v2 = mesh->to_vertex_handle(he0h);

		// 	const auto& pt1 = mesh->point(v1);
		// 	const auto& pt2 = mesh->point(v2);

		// 	glVertex3f(pt1[0], pt1[1], pt1[2]);
		// 	glVertex3f(pt2[0], pt2[1], pt2[2]);
		// }
		// glEnd();

		// glEnable(GL_LIGHTING);

		// iterate over faces
		for(auto v_it = mesh->faces_begin(); v_it != mesh->faces_end(); ++v_it) {
			glBegin(GL_TRIANGLE_FAN);

			// per-face normal - only used if no other normals are available
			if(mesh->has_face_normals() && !mesh->has_halfedge_normals() && !mesh->has_vertex_normals()) {
				const auto& norm = mesh->normal(*v_it);
				glNormal3f(norm[0], norm[1], norm[2]);
			}

			// iterate over halfedges
			for(auto he_it = mesh->cfh_iter(*v_it); he_it.is_valid(); ++he_it) {
				// get the vertexhandle for "to" vertex
				const auto& pth = mesh->to_vertex_handle(*he_it);

				// use halfedge normal if available, or vertex normal as a second choice
				if(mesh->has_halfedge_normals()) {
					const auto& norm = mesh->normal(*he_it);
					glNormal3f(norm[0], norm[1], norm[2]);
				}
				else if(mesh->has_vertex_normals()) {
					const auto& norm = mesh->normal(pth);
					glNormal3f(norm[0], norm[1], norm[2]);
				}

				// and draw the vertex
				const auto& pt = mesh->point(pth);
				glVertex3f(pt[0], pt[1], pt[2]);
			}

			glEnd();
		}

		glEnd();
	}
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_mesh, "mesh");

	meta.setDraw(draw);
}

possumwood::NodeImplementation s_impl("openmesh/display", init);

}
