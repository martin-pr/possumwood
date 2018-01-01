#include "grid.h"

#include <vector>
#include <cassert>

namespace possumwood {

namespace {

const std::vector<Imath::V3f>& makeGrid() {
	static std::vector<Imath::V3f> result;

	if(result.empty()) {
		for(int a = -10; a <= 10; ++a) {
			result.push_back(Imath::V3f(a, -10.0f, a % 10 == 0));
			result.push_back(Imath::V3f(a, 10.0f, a % 10 == 0));
			result.push_back(Imath::V3f(-10.0f, a, a % 10 == 0));
			result.push_back(Imath::V3f(10.0f, a, a % 10 == 0));
		}
	}

	return result;
}

const std::string& vertexShaderSource() {
	static const std::string s_source =
	    " \
		#version 150 \n \
		in vec3 in_Position; \n \
		uniform mat4 in_Projection; \n \
		uniform mat4 in_Modelview; \n \
		out float vert_color; \n \
		\n \
		void main(void) { \n \
		    gl_Position = in_Projection * in_Modelview * vec4(in_Position.x, 0.0, in_Position.y, 1.0); \n \
		    vert_color = in_Position.z; \n \
		}";

	return s_source;
}

const std::string& fragmentShaderSource() {
	static const std::string s_source =
	    " \
		#version 150 \n \
		in float vert_color; \n \
		out vec4 out_color; \n \
		\n \
		void main(void) { \n \
			float c = 0.5*vert_color + 0.5; \n \
		    out_color = vec4(c,c,c,1.0); \n \
		}";

	return s_source;
}
}

Grid::Grid() : GLRenderable(vertexShaderSource(), fragmentShaderSource()) {
	possumwood::GLRenderable::VBO vbo = updateVertexData();

	vbo.data = makeGrid();
	vbo.drawType = GL_LINES;
}
}
