#include "vertex_data.inl"

#include <possumwood_sdk/app.h>

#include <limits>

namespace possumwood {

VertexData::VertexData(GLenum drawElementType) : m_drawElementType(drawElementType) {
}

dependency_graph::State VertexData::use(GLuint programId, const ViewportState& vs) const {
	dependency_graph::State state;

	// do the updates, if needed
	for(auto& v : m_vbos)
		if(v.updateType == kPerDraw || v.buffer == nullptr)
			v.buffer = v.update(vs);

	// and try to use each
	for(auto& v : m_vbos) {
		GLint attrLocation = glGetAttribLocation(programId, v.name.c_str());
		if(attrLocation >= 0)
			v.vbo->use(attrLocation);
		// else
		// 	state.addWarning("VBO '" + v.name + "' cannot be mapped to an attribute location - not used in any of the
		// programs?");
	}

	return state;
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

std::set<std::string> VertexData::names() const {
	std::set<std::string> result;

	for(auto& vbo : m_vbos)
		result.insert(vbo.name);

	return result;
}

std::ostream& operator<<(std::ostream& out, const VertexData& vd) {
	out << "vertex data with " << vd.vboCount() << " VBOs:" << std::endl;
	for(auto& v : vd.m_vbos)
		out << "  - " << v.glslType << std::endl;

	return out;
}

}  // namespace possumwood
