#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/metadata.inl>
#include <possumwood_sdk/gl_renderable.h>

#include <GL/glut.h>

#include "datatypes/skeleton.h"

namespace {

dependency_graph::InAttr<anim::Skeleton> a_skel;

const std::string& fragmentShaderSource() {
	static const std::string s_source =
	    " \
		#version 150 \n \
		out vec4 out_color; \n \
		\n \
		void main(void) { \n \
		    out_color = vec4(1,0,0,1.0); \n \
		}";

	return s_source;
}

class Skeleton : public possumwood::Drawable {
  public:
	Skeleton(dependency_graph::Values&& vals)
	    : possumwood::Drawable(std::move(vals)),
	      m_renderable(GL_LINES, possumwood::GLRenderable::defaultVertexShader(),
	                   fragmentShaderSource()) {
		m_timeChangedConnection =
		    possumwood::App::instance().onTimeChanged([this](float t) { refresh(); });
	}

	~Skeleton() {
		m_timeChangedConnection.disconnect();
	}

	dependency_graph::State draw() {
		anim::Skeleton skel = values().get(a_skel);

		{
			auto vbo = m_renderable.updateVertexData();

			if(skel.size() > 1) {
				// convert to world space
				for(auto& j : skel)
					if(j.hasParent())
						j.tr() = j.parent().tr() * j.tr();

				// and draw the result
				vbo.data.resize((skel.size() - 1) * 2);

				unsigned counter = 0;
				for(auto& j : skel)
					if(j.hasParent()) {
						vbo.data[counter++] = j.parent().tr().translation;
						vbo.data[counter++] = j.tr().translation;
					}
				assert(counter == (skel.size() - 1) * 2);
			}
			else
				vbo.data.clear();
		}

		m_renderable.draw(viewport().projection, viewport().modelview);

		return dependency_graph::State();
	}

  private:
	possumwood::GLRenderable m_renderable;

	boost::signals2::connection m_timeChangedConnection;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_skel, "skeleton");

	meta.setDrawable<Skeleton>();
}

possumwood::NodeImplementation s_impl("anim/frame/display", init);
}
