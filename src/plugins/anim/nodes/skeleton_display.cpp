#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/metadata.inl>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include <GL/glut.h>

#include "io/animation.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const anim::Animation>> a_anim;
dependency_graph::InAttr<bool> a_showBase;

struct Drawable : public possumwood::Drawable {
	Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)) {
		m_timeChangedConnection = possumwood::App::instance().onTimeChanged([this](float t) {
			refresh();
		});
	}

	~Drawable() {
		m_timeChangedConnection.disconnect();
	}

	void draw() {
		const std::shared_ptr<const anim::Animation> anim = values().get(a_anim);
		const bool showBase = values().get(a_showBase);

		if(anim) {
			glColor3f(1, 0, 0);

			glDisable(GL_LIGHTING);

			glBegin(GL_LINES);

			anim::Skeleton frame;
			if(showBase || anim->frames.empty())
				frame = anim->base;
			else {
				std::size_t frameId = std::round(possumwood::App::instance().time() * anim->fps);
				frameId = std::max(std::size_t(0), std::min(anim->frames.size()-1, frameId));

				frame = anim->frames[frameId];
			}

			// convert to world space
			for(auto& j : frame)
				if(j.hasParent())
					j.tr() = j.parent().tr() * j.tr();

			// and draw the result
			for(auto& j : frame)
				if(j.hasParent()) {
					glVertex3fv(j.parent().tr().translation.getValue());
					glVertex3fv(j.tr().translation.getValue());
				}

			glEnd();

			glEnable(GL_LIGHTING);
		}
	}

	boost::signals2::connection m_timeChangedConnection;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_anim, "anim");
	meta.addAttribute(a_showBase, "base_skeleton", true);

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("anim/skeleton_display", init);

}
