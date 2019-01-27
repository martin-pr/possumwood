#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <OpenEXR/ImathVec.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include <possumwood_sdk/gl_renderable.h>

#include "io/vec3.h"

namespace {

dependency_graph::InAttr<Imath::Vec3<float>> a_pos, a_size;

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

class Cube : public possumwood::Drawable {
  public:
	Cube(dependency_graph::Values&& vals)
	    : possumwood::Drawable(std::move(vals)),
	      m_renderable(GL_LINES, possumwood::GLRenderable::defaultVertexShader(),
	                   fragmentShaderSource()) {
		// just feed the vertex data of a unit cube into the VBO
		auto vbo = m_renderable.updateVertexData();

		vbo.data.push_back(Imath::V3f(-1, -1, -1));
		vbo.data.push_back(Imath::V3f(1, -1, -1));
		vbo.data.push_back(Imath::V3f(1, -1, -1));
		vbo.data.push_back(Imath::V3f(1, 1, -1));
		vbo.data.push_back(Imath::V3f(1, 1, -1));
		vbo.data.push_back(Imath::V3f(-1, 1, -1));
		vbo.data.push_back(Imath::V3f(-1, 1, -1));
		vbo.data.push_back(Imath::V3f(-1, -1, -1));

		vbo.data.push_back(Imath::V3f(-1, -1, 1));
		vbo.data.push_back(Imath::V3f(1, -1, 1));
		vbo.data.push_back(Imath::V3f(1, -1, 1));
		vbo.data.push_back(Imath::V3f(1, 1, 1));
		vbo.data.push_back(Imath::V3f(1, 1, 1));
		vbo.data.push_back(Imath::V3f(-1, 1, 1));
		vbo.data.push_back(Imath::V3f(-1, 1, 1));
		vbo.data.push_back(Imath::V3f(-1, -1, 1));

		vbo.data.push_back(Imath::V3f(-1, -1, 1));
		vbo.data.push_back(Imath::V3f(-1, -1, -1));
		vbo.data.push_back(Imath::V3f(1, -1, 1));
		vbo.data.push_back(Imath::V3f(1, -1, -1));
		vbo.data.push_back(Imath::V3f(1, 1, 1));
		vbo.data.push_back(Imath::V3f(1, 1, -1));
		vbo.data.push_back(Imath::V3f(-1, 1, 1));
		vbo.data.push_back(Imath::V3f(-1, 1, -1));
	}

  protected:
	virtual dependency_graph::State draw() {
		// get the original modelview
		Imath::M44f modelview = viewport().modelview();

		const Imath::Vec3<float> pos = values().get(a_pos);
		const Imath::Vec3<float> size = values().get(a_size);

		modelview = Imath::M44f(size.x, 0, 0, 0, 0, size.y, 0, 0, 0, 0, size.z, 0, pos.x,
		                        pos.y, pos.z, 1) *
		            modelview;

		m_renderable.draw(viewport().projection(), modelview);

		return dependency_graph::State();
	}

  private:
	possumwood::GLRenderable m_renderable;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_pos, "position", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_size, "size", Imath::Vec3<float>(1, 1, 1));

	meta.setDrawable<Cube>();
}

possumwood::NodeImplementation s_impl("maths/cube", init);
}
