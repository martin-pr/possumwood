#include "vertex_data.inl"

#include <possumwood_sdk/app.h>

#include <limits>

namespace possumwood {

VertexData::VertexData(GLenum drawElementType) : m_drawElementType(drawElementType) {
}

void VertexData::use(GLuint programId, const Drawable::ViewportState& vs) const {
	// do the updates, if needed
	for(auto& v : m_vbos)
		if(v.updateType == kPerDraw || v.buffer == nullptr)
			v.buffer = v.update(vs);

	// and try to use each
	for(auto& v : m_vbos) {
		GLint attrLocation = glGetAttribLocation(programId, v.name.c_str());
		if(attrLocation >= 0)
			v.vbo->use(attrLocation);
	}
}

GLenum VertexData::drawElementType() const {
	return m_drawElementType;
}

std::size_t VertexData::size() const {
	if(m_vbos.empty())
		return 0;

	return m_vbos[0].size;
}

std::string VertexData::glslDeclaration() const {
	std::stringstream result;

	for(auto& a : m_vbos)
		result << a.glslType << std::endl;

	return result.str();
}

std::size_t VertexData::vboCount() const {
	return m_vbos.size();
}
}
