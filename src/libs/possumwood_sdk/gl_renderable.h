#pragma once

#include <vector>
#include <map>

#include <GL/glew.h>
#include <GL/gl.h>

#include <boost/noncopyable.hpp>

#include <ImathMatrix.h>
#include <ImathVec.h>

namespace possumwood {

/// A very simple encapsulation of GL Core Profile renderable object, to simplify the API
/// to use something similar to the old GL functionality. Not really intended to be used
/// for complex objects - there are other ways of rendering more complex objects in the
/// Render plugin.
class GLRenderable : public boost::noncopyable {
  public:
	static const std::string& defaultVertexShader();
	static const std::string& defaultFragmentShader();

	GLRenderable(GLenum drawType,
	             const std::string& vertexShaderSrc = defaultVertexShader(),
	             const std::string& fragmentShaderSrc = defaultFragmentShader());
	virtual ~GLRenderable();

	void draw(const Imath::M44f& projection, const Imath::M44f& modelview);

	class VBO : public boost::noncopyable {
	  public:
		VBO(VBO&& src);
		~VBO();

		std::vector<Imath::V3f> data;

	  private:
		VBO(GLRenderable* parent, const std::string& name);

		GLRenderable* m_parent;
		const std::string m_name;

		friend class GLRenderable;
	};

	/// returns a scoped object allowing to update vertex data. On this object's
	/// destruction, the data are moved to be held by GLRenderable.
	VBO updateVertexData(const std::string& attrName = "in_Position");

  private:
	struct VBOData {
		bool needsUpdate = true;
		std::vector<Imath::V3f> data;
		GLuint VBOId = 0;
	};

	/// initialises the OpenGL setup (except VBO)
	void initialise();
	/// initialises VBO data (assumes initialise() has been called beforehand, and a valid
	/// VAO is bound)
	void updateVBO(GLuint index, VBOData& data);

	GLuint m_vao;
	GLuint m_vertexShader, m_fragmentShader, m_program;

	std::string m_vertexShaderSrc, m_fragmentShaderSrc;

	GLenum m_drawType;

	std::map<std::string, VBOData> m_vbos;

	friend class VBO;
};
}
