#pragma once

#include <ImathMatrix.h>
#include <ImathVec.h>

#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

namespace possumwood {

/// A very simple encapsulation of GL Core Profile renderable object, to simplify the API
/// to use something similar to the old GL functionality. Not really intended to be used
/// for complex objects - there are other ways of rendering more complex objects in the
/// Render plugin.
class GLRenderable : public boost::noncopyable {
  public:
	static const std::string& defaultVertexShader();
	static const std::string& defaultFragmentShader();

	GLRenderable(unsigned drawType, const std::string& vertexShaderSrc = defaultVertexShader(),
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

	/// sets a single uniform value.
	/// TODO: generalize to other data types
	void setUniform(const std::string& name, const Imath::V3f& value);

  private:
	struct VBOData {
		bool needsUpdate = true;
		std::vector<Imath::V3f> data;
		unsigned VBOId = 0;
	};

	/// initialises the OpenGL setup (except VBO)
	void initialise();
	/// initialises VBO data (assumes initialise() has been called beforehand, and a valid
	/// VAO is bound)
	void updateVBO(unsigned index, VBOData& data);

	unsigned m_vao;
	unsigned m_vertexShader, m_fragmentShader, m_program;

	std::string m_vertexShaderSrc, m_fragmentShaderSrc;

	unsigned m_drawType;

	std::map<std::string, VBOData> m_vbos;
	std::map<std::string, Imath::V3f> m_uniforms;

	friend class VBO;
};
}  // namespace possumwood
