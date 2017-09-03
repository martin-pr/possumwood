#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/metadata.inl>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include <GL/glut.h>

#include "datatypes/skinned_mesh.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_meshes;

struct Drawable : public possumwood::Drawable {
	Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)) {
	}

	void draw() {
		std::shared_ptr<const std::vector<anim::SkinnedMesh>> meshes = values().get(a_meshes);

		if(meshes != nullptr) {
			glColor3f(1, 1, 1);

			glPushAttrib(GL_ALL_ATTRIB_BITS);

			glEnable(GL_LIGHTING);
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

			glBegin(GL_TRIANGLES);

			for(auto& mesh : *meshes) {
				std::size_t ctr = 0;
				for(auto& poly : mesh.polygons()) {
					const Imath::V3f& v1 = mesh.vertices()[poly[0]].pos();
					const Imath::V3f& v2 = mesh.vertices()[poly[1]].pos();
					const Imath::V3f& v3 = mesh.vertices()[poly[2]].pos();

					// const Imath::V3f norm = (v2-v1).cross(v3-v1).normalized();
					// glNormal3fv(&norm[0]);

					glNormal3fv(&mesh.normals()[ctr++][0]);
					glVertex3fv(&v1[0]);
					glNormal3fv(&mesh.normals()[ctr++][0]);
					glVertex3fv(&v2[0]);
					glNormal3fv(&mesh.normals()[ctr++][0]);
					glVertex3fv(&v3[0]);
				}
			}

			glEnd();

			glPopAttrib();
		}
	}
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_meshes, "meshes");

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("anim/meshes_display", init);

}
