#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/metadata.inl>

#include <GL/gl.h>

#include "datatypes/skeleton.h"

namespace {

dependency_graph::InAttr<anim::Skeleton> a_skel;

struct Drawable : public possumwood::Drawable {
	Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)) {
		m_timeChangedConnection = possumwood::App::instance().onTimeChanged([this](float t) {
			refresh();
		});
	}

	~Drawable() {
		m_timeChangedConnection.disconnect();
	}

	dependency_graph::State draw() {
		anim::Skeleton skel = values().get(a_skel);

		if(!skel.empty()) {
			glColor3f(1, 0, 0);

			glDisable(GL_LIGHTING);

			glBegin(GL_LINES);

			// convert to world space
			for(auto& j : skel)
				if(j.hasParent())
					j.tr() = j.parent().tr() * j.tr();

			// and draw the result
			for(auto& j : skel)
				if(j.hasParent()) {
					glVertex3fv(j.parent().tr().translation.getValue());
					glVertex3fv(j.tr().translation.getValue());
				}

			glEnd();

			glEnable(GL_LIGHTING);
		}

		return dependency_graph::State();
	}

	boost::signals2::connection m_timeChangedConnection;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_skel, "skeleton");

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("anim/frame/display", init);

}
