#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>
#include <possumwood_sdk/app.h>

#include <OpenEXR/ImathVec.h>

#include <GL/glut.h>

#include "io/vec3.h"

namespace {

dependency_graph::InAttr<Imath::Vec3<float>> a_pos, a_size;

struct Drawable : public possumwood::Drawable {
	Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)) {
		m_timeChangedConnection = possumwood::App::instance().onTimeChanged([this](float t) {
			refresh();
		});
	}

	void draw() {
		const Imath::Vec3<float> pos = values().get(a_pos);
		const Imath::Vec3<float> size = values().get(a_size);

		glPushMatrix();

		glTranslatef(pos.x, pos.y, pos.z);
		glScalef(size.x, size.y, size.z);

		glDisable(GL_LIGHTING);
		glColor3f(1, 0, 0);
		glRotatef(possumwood::App::instance().time(), 0,1,0);
		glutWireCube(0.5f);
		glEnable(GL_LIGHTING);

		glPopMatrix();
	}

	boost::signals2::connection m_timeChangedConnection;
};


void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_pos, "position", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_size, "size", Imath::Vec3<float>(1, 1, 1));

	meta.setDrawableFactory([](dependency_graph::Values&& vals) {
		return std::unique_ptr<possumwood::Drawable>(
			new Drawable(std::move(vals)));
	});
}

possumwood::NodeImplementation s_impl("3d/cube", init);

}
