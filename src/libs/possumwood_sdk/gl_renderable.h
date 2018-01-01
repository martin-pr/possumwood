#pragma once

#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>

#include <boost/noncopyable.hpp>

#include <ImathMatrix.h>
#include <ImathVec.h>

namespace possumwood {

/// A very simple encapsulation of GL Core Profile renderable object, to simplify the API
/// to use something similar to the old GL functionality.
class GLRenderable : public boost::noncopyable {
  public:
	GLRenderable();
	virtual ~GLRenderable();

	void draw(const Imath::M44f& projection, const Imath::M44f& modelview);

	class VBO : public boost::noncopyable {
	  public:
		VBO(VBO&& src);
		~VBO();

		std::vector<Imath::V3f> data;
		GLenum drawType;

	  private:
		VBO(GLRenderable* parent);

		GLRenderable* m_parent;

		friend class GLRenderable;
	};

	/// returns a scoped object allowing to update vertex data. On this object's
	/// destruction, the data are moved to be held by GLRenderable.
	VBO updateVertexData();

  protected:
	virtual const std::string& vertexShaderSource() const;
	virtual const std::string& fragmentShaderSource() const;

  private:
	/// initialises the OpenGL setup (except VBO)
	void initialise();
	/// initialises VBO data (assumes initialise() has been called beforehand, and a valid
	/// VAO is bound)
	void updateVBO();

	GLuint m_vao, m_verticesVBO;
	GLuint m_vertexShader, m_fragmentShader, m_program;

	bool m_needsVBOUpdate;
	std::vector<Imath::V3f> m_vboData;
	GLenum m_drawType;

	friend class VBO;
};
}
