#include "uniforms.inl"

#include <limits>
#include <regex>

#include <possumwood_sdk/app.h>

namespace possumwood {

Uniforms::Uniforms() : m_currentTime(std::numeric_limits<float>::infinity()) {
}

void Uniforms::addTexture(const std::string& name, const QPixmap& pixmap) {
	m_textures.push_back(TextureHolder());

	m_textures.back().name = name;
	m_textures.back().glslType = "uniform sampler2D " + name + ";";
	m_textures.back().texture = std::shared_ptr<const Texture>(new Texture(pixmap));
}

void Uniforms::use(GLuint programId, const Drawable::ViewportState& vs) const {
	const bool timeUpdate = m_currentTime != possumwood::App::instance().time();
	m_currentTime = possumwood::App::instance().time();

	for(auto& u : m_uniforms) {
		if(u.updateType == kPerDraw || (timeUpdate && u.updateType == kPerFrame) || !u.initialised) {
			u.updateFunctor(*u.data, vs);
			u.initialised = true;
		}

		u.useFunctor(programId, u.name, *u.data);
	}

	for(unsigned tex = 0; tex < m_textures.size(); ++tex) {
		GLint attr = glGetUniformLocation(programId, m_textures[tex].name.c_str());
		if(attr >= 0)
			m_textures[tex].texture->use(attr, GL_TEXTURE0 + tex);
	}
}

std::size_t Uniforms::size() const {
	return m_uniforms.size() + m_textures.size();
}

namespace {
	std::pair<std::string, std::size_t> glslVariableSplit(const std::string& var) {
		assert(var.length() > 1);
		assert(var.back() == ';');

		static const std::regex s_regex("^([^[]+)\\[([0-9]+)\\];$");
		std::smatch match;

		if(std::regex_match(var, match, s_regex)) {
			assert(match.size() == 3);
			return std::make_pair(match[1], std::atoi(std::string(match[2]).c_str()));
		}
		else
			// remove the last ;
			return std::make_pair(var.substr(0, var.length()-1), 1);
	}

	template<typename CONTAINER>
	void processGLSLNames(std::map<std::string, std::size_t>& vars, const CONTAINER& data) {
		for(auto& item : data) {
			auto split = glslVariableSplit(item.glslType);

			auto it = vars.find(split.first);
			if(it == vars.end())
				vars.insert(split);
			else
				it->second = std::max(it->second, split.second);
		}

	}
}


std::string Uniforms::glslDeclaration() const {
	// get a list of variables with their sizes
	std::map<std::string, std::size_t> vars;

	processGLSLNames(vars, m_uniforms);
	processGLSLNames(vars, m_textures);

	// and convert them into a string
	std::stringstream result;
	for(auto& a : vars)
		if(a.second > 1)
			result << a.first << "[" << a.second << "];" << std::endl;
		else
			result << a.first << ";" << std::endl;

	return result.str();
}

}
