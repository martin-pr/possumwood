#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <OpenEXR/ImathVec.h>

#include <GL/gl.h>

#include "io/vec3.h"

namespace {

dependency_graph::InAttr<Imath::Vec3<float>> a_pos, a_size;

dependency_graph::State draw(const dependency_graph::Values& values, const possumwood::Drawable::ViewportState& viewport) {
	const Imath::Vec3<float> pos = values.get(a_pos);
	const Imath::Vec3<float> size = values.get(a_size);

	glPushMatrix();

	glTranslatef(pos.x, pos.y, pos.z);
	glScalef(size.x, size.y, size.z);

	glDisable(GL_LIGHTING);
	glColor3f(1, 0, 0);

	glBegin(GL_LINES);

	glVertex3f(-1, -1, -1); glVertex3f(1, -1, -1);
	glVertex3f(1, -1, -1); glVertex3f(1, 1, -1);
	glVertex3f(1, 1, -1); glVertex3f(-1, 1, -1);
	glVertex3f(-1, 1, -1); glVertex3f(-1, -1, -1);

	glVertex3f(-1, -1, 1); glVertex3f(1, -1, 1);
	glVertex3f(1, -1, 1); glVertex3f(1, 1, 1);
	glVertex3f(1, 1, 1); glVertex3f(-1, 1, 1);
	glVertex3f(-1, 1, 1); glVertex3f(-1, -1, 1);

	glVertex3f(-1, -1, 1); glVertex3f(-1, -1, -1);
	glVertex3f(1, -1, 1); glVertex3f(1, -1, -1);
	glVertex3f(1, 1, 1); glVertex3f(1, 1, -1);
	glVertex3f(-1, 1, 1); glVertex3f(-1, 1, -1);

	glEnd();

	glEnable(GL_LIGHTING);

	glPopMatrix();

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_pos, "position", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_size, "size", Imath::Vec3<float>(1, 1, 1));

	meta.setDrawable(&draw);
}

possumwood::NodeImplementation s_impl("maths/cube", init);

}
