#include "uniforms.inl"

#include <limits>

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

void Uniforms::use(GLuint programId, const ViewportState& vs) const {
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

std::string Uniforms::glslDeclaration() const {
	std::stringstream result;

	for(auto& u : m_uniforms)
		result << u.glslType << std::endl;

	for(auto& t : m_textures)
		result << t.glslType << std::endl;

	return result.str();
}

}
