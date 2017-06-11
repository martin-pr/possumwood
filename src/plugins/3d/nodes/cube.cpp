#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include <OpenEXR/ImathVec.h>

#include <GL/glut.h>

#include "io/vec3.h"

namespace {

dependency_graph::InAttr<Imath::Vec3<float>> a_pos, a_size;

void draw(const dependency_graph::Values& data) {
	const Imath::Vec3<float> pos = data.get(a_pos);
	const Imath::Vec3<float> size = data.get(a_size);

	glPushMatrix();

	glTranslatef(pos.x, pos.y, pos.z);
	glScalef(size.x, size.y, size.z);

	glColor3f(1, 0, 0);
	glutWireCube(0.5f);

	glPopMatrix();
}

void init(Metadata& meta) {
	meta.addAttribute(a_pos, "position", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_size, "size", Imath::Vec3<float>(1, 1, 1));

	meta.setDraw(draw);
}

NodeImplementation s_impl("3d/cube", init);

}
