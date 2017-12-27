#pragma once

#include <boost/noncopyable.hpp>

#include <GL/glew.h>
#include <GL/gl.h>

#include <ImathMatrix.h>

namespace possumwood {

/// A simple drawable grid, compatible with Core GL profile. To be created during a draw
/// call, when a GL context is active.
class Grid : public boost::noncopyable {
  public:
	Grid();

	void draw(const Imath::M44f& projection, const Imath::M44f& modelview);

  private:
  	GLuint m_vao, m_verticesVBO;
  	GLuint m_vertexShader, m_fragmentShader, m_program;
};
}
