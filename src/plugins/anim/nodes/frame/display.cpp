#include <GL/glew.h>

#include <GL/gl.h>

#include <maths/io/vec3.h>

#include <possumwood_sdk/app.h>
#include <possumwood_sdk/gl.h>
#include <possumwood_sdk/gl_renderable.h>
#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/metadata.inl>

#include "datatypes/skeleton.h"

namespace {

dependency_graph::InAttr<anim::Skeleton> a_skel;
dependency_graph::InAttr<Imath::V3f> a_colour;

const std::string& fragmentShaderSource() {
	static const std::string s_source =
	    " \
		#version 150 \n \
		uniform vec3 colour; \n \
		out vec4 out_color; \n \
		\n \
		void main(void) { \n \
		    out_color = vec4(colour, 1.0); \n \
		}";

	return s_source;
}

class Skeleton : public possumwood::Drawable {
  public:
	Skeleton(dependency_graph::Values&& vals)
	    : possumwood::Drawable(std::move(vals)),
	      m_renderable(GL_LINES, possumwood::GLRenderable::defaultVertexShader(), fragmentShaderSource()) {
		m_timeChangedConnection = possumwood::App::instance().onTimeChanged([](float t) { refresh(); });
	}

	~Skeleton() {
		m_timeChangedConnection.disconnect();
	}

	dependency_graph::State draw() {
		GL_CHECK_ERR;

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

		GL_CHECK_ERR;

		m_renderable.setUniform("colour", values().get(a_colour));

		GL_CHECK_ERR;

		m_renderable.draw(viewport().projection(), viewport().modelview());

		GL_CHECK_ERR;

		return dependency_graph::State();
	}

  private:
	possumwood::GLRenderable m_renderable;

	boost::signals2::connection m_timeChangedConnection;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_skel, "skeleton", anim::Skeleton(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_colour, "colour", Imath::V3f(1, 1, 1));

	meta.setDrawable<Skeleton>();
}

possumwood::NodeImplementation s_impl("anim/frame/display", init);
}  // namespace
