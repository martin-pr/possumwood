#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/metadata.inl>
#include <possumwood_sdk/gl_renderable.h>
#include <possumwood_sdk/gl.h>

#include <GL/glut.h>

#include <maths/io/vec3.h>

#include "datatypes/constraints.h"

namespace {

dependency_graph::InAttr<anim::Constraints> a_constraints;
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

const std::string& vertexShaderSource() {
	static std::string s_src =
		"#version 130 \n"
		" \n"
		"in vec3 P;                     // position attr from the vbo \n"
		" \n"
		"uniform mat4 in_Projection;      // projection matrix \n"
		"uniform mat4 in_Modelview;       // modelview matrix \n"
		" \n"
		"out vec3 vertexPosition;       // vertex position for the fragment shader \n"
		" \n"
		"void main() {\n"
		"	vec4 pos4 = vec4(P.x, P.y, P.z, 1);\n"
		"\n"
		"	vertexPosition = (in_Modelview * pos4).xyz;\n"
		"   	gl_Position = in_Projection * in_Modelview * pos4;\n"
		"   gl_PointSize = 5;\n"
		"} \n";

	return s_src;
}


class Drawable : public possumwood::Drawable {
  public:
	Drawable(dependency_graph::Values&& vals)
	    : possumwood::Drawable(std::move(vals)), m_renderable(GL_POINTS, vertexShaderSource(), fragmentShaderSource()) {
	}

	~Drawable() {
	}

	dependency_graph::State draw() {
		GL_CHECK_ERR;

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

		const anim::Constraints& constraints = values().get(a_constraints);

		std::size_t totalCount = 0;
		for(auto& c : constraints)
			totalCount += c.second.size();

		{
			auto vbo = m_renderable.updateVertexData();

			if(totalCount > 0) {

				vbo.data.resize(totalCount);

				std::size_t ctr = 0;
				for(auto& c : constraints)
					for(auto& i : c.second) {
						vbo.data[ctr] = i.origin().translation;

						++ctr;
					}
			}
			else
				vbo.data.clear();
		}

		GL_CHECK_ERR;

		m_renderable.setUniform("colour", values().get(a_colour));

		GL_CHECK_ERR;

		m_renderable.draw(viewport().projection(), viewport().modelview());

		GL_CHECK_ERR;

		glDisable(GL_POINT_SPRITE);
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

		GL_CHECK_ERR;

		return dependency_graph::State();
	}

  private:
	possumwood::GLRenderable m_renderable;

	boost::signals2::connection m_timeChangedConnection;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_constraints, "constraints", anim::Constraints(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_colour, "colour", Imath::V3f(1,1,1));

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("anim/constraints/display", init);
}
