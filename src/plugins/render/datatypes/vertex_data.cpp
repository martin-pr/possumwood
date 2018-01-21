#include "vertex_data.inl"

#include <possumwood_sdk/app.h>

#include <limits>
#include <regex>

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

namespace {
	std::pair<std::string, std::size_t> glslVariableSplit(const std::string& var) {
		assert(var.length() > 1);
		assert(var.back() == ';');

		static const std::regex s_regex("^([^[]+)\\[([0-9]+)\\];$");
		std::smatch match;

		if(std::regex_match(var, match, s_regex)) {
			assert(match.size() == 3);
			return std::make_pair(match[1], std::atoi(std::string(match[2]).c_str())+1);
		}
		else
			// remove the last ;
			return std::make_pair(var.substr(0, var.length()-1), 1);
	}
}

std::string VertexData::glslDeclaration() const {
	// get a list of variables with their sizes
	std::map<std::string, std::size_t> vars;
	for(auto& vbo : m_vbos) {
		auto split = glslVariableSplit(vbo.glslType);

		auto it = vars.find(split.first);
		if(it == vars.end())
			vars.insert(split);
		else
			it->second = std::max(it->second, split.second);
	}

	std::stringstream result;
	for(auto& a : vars)
		if(a.second > 1)
			result << a.first << "[" << a.second << "];" << std::endl;
		else
			result << a.first << ";" << std::endl;

	return result.str();
}

std::size_t VertexData::vboCount() const {
	return m_vbos.size();
}
}
