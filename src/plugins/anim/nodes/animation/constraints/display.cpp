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
dependency_graph::InAttr<bool> a_showTrajectory, a_showConstrained;

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
	    : possumwood::Drawable(std::move(vals)), m_points(GL_POINTS, vertexShaderSource(), fragmentShaderSource()), m_lines(GL_LINES, vertexShaderSource(), fragmentShaderSource()) {
	}

	~Drawable() {
	}

	dependency_graph::State draw() {
		GL_CHECK_ERR;

		{
			// possumwood::ScopedEnable sprite(GL_POINT_SPRITE); // removed in 3.2
			possumwood::ScopedEnable point_size(GL_VERTEX_PROGRAM_POINT_SIZE);

			GL_CHECK_ERR;

			const anim::Constraints& constraints = values().get(a_constraints);

			std::size_t pointCount = 0, lineCount = 0;
			for(auto& c : constraints) {
				pointCount += c.second.size();

				if(values().get(a_showTrajectory) && !c.second.frames().empty())
					lineCount += c.second.frames().size() - 1;

				else if(values().get(a_showConstrained))
					for(auto& i : c.second)
						lineCount += i.endFrame() - i.startFrame();
			}

			{
				auto vbo = m_points.updateVertexData();

				if(pointCount > 0) {

					vbo.data.resize(pointCount);

					std::size_t ctr = 0;
					for(auto& c : constraints)
						for(auto& i : c.second)
							vbo.data[ctr++] = i.origin().translation;
				}
				else
					vbo.data.clear();
			}

			{
				auto vbo = m_lines.updateVertexData();

				if(lineCount > 0) {
					vbo.data.resize(lineCount * 2);

					std::size_t ctr = 0;
					for(auto& c : constraints)
						if(values().get(a_showTrajectory)) {
							auto it2 = c.second.frames().begin();
							auto it = it2;
							++it2;

							vbo.data[ctr++] = it->tr().translation;
							++it;
							++it2;

							while(it2 != c.second.frames().end()) {
								vbo.data[ctr++] = it->tr().translation;
								vbo.data[ctr++] = it->tr().translation;

								++it;
								++it2;
							}
							vbo.data[ctr++] = it->tr().translation;
						}

						else if(values().get(a_showConstrained))
							for(auto& i : c.second)
								for(std::size_t fr=i.startFrame(); fr<i.endFrame(); ++fr) {
									vbo.data[ctr++] = c.second.frames()[fr].tr().translation;
									vbo.data[ctr++] = c.second.frames()[fr+1].tr().translation;
								}

					assert(ctr == lineCount * 2);
				}
				else
					vbo.data.clear();
			}

			GL_CHECK_ERR;

			m_points.setUniform("colour", values().get(a_colour));
			m_lines.setUniform("colour", values().get(a_colour));

			GL_CHECK_ERR;

			m_points.draw(viewport().projection(), viewport().modelview());
			m_lines.draw(viewport().projection(), viewport().modelview());

			GL_CHECK_ERR;
		}

		GL_CHECK_ERR;

		return dependency_graph::State();
	}

  private:
	possumwood::GLRenderable m_points, m_lines;

	boost::signals2::connection m_timeChangedConnection;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_constraints, "constraints", anim::Constraints(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_colour, "colour", Imath::V3f(1,1,1));
	meta.addAttribute(a_showTrajectory, "show_trajectory", false);
	meta.addAttribute(a_showConstrained, "show_constrained", true);

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("anim/constraints/display", init);
}
