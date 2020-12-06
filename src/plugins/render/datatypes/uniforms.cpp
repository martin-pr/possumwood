#include "uniforms.inl"

#include <possumwood_sdk/app.h>
#include <possumwood_sdk/gl.h>

#include <limits>

namespace possumwood {

Uniforms::Uniforms() : m_currentTime(std::numeric_limits<float>::infinity()) {
}

void Uniforms::addTexture(const std::string& name,
                          const unsigned char* data,
                          std::size_t width,
                          std::size_t height,
                          const Texture::Format& format) {
	m_textures.push_back(TextureHolder());

	m_textures.back().name = name;
	m_textures.back().glslType = "uniform sampler2D " + name + ";";
	m_textures.back().texture = std::shared_ptr<const Texture>(new Texture(data, width, height, format));
}

void Uniforms::addTexture(const std::string& name,
                          const float* data,
                          std::size_t width,
                          std::size_t height,
                          const Texture::Format& format) {
	m_textures.push_back(TextureHolder());

	m_textures.back().name = name;
	m_textures.back().glslType = "uniform sampler2D " + name + ";";
	m_textures.back().texture = std::shared_ptr<const Texture>(new Texture(data, width, height, format));
}

void Uniforms::addTextureArray(const std::string& name,
                               std::vector<const unsigned char*> data,
                               std::size_t width,
                               std::size_t height,
                               const Texture::Format& format) {
	m_textures.push_back(TextureHolder());

	m_textures.back().name = name;
	m_textures.back().glslType = "uniform sampler2DArray " + name + ";";
	m_textures.back().texture = std::shared_ptr<const Texture>(new Texture(data, width, height, format));
}

dependency_graph::State Uniforms::use(GLuint programId, const ViewportState& vs) const {
	dependency_graph::State state;

	const bool timeUpdate = m_currentTime != possumwood::App::instance().time();
	m_currentTime = possumwood::App::instance().time();

	for(auto& u : m_uniforms) {
		if(u->updateType == kPerDraw || (timeUpdate && u->updateType == kPerFrame) || !u->initialised) {
			u->updateFunctor(*u->data, vs);
			u->initialised = true;
		}

		GL_CHECK_ERR;

		dependency_graph::State tmp = u->useFunctor(programId, u->name, *u->data);
		state.append(tmp);

		GL_CHECK_ERR;
	}

	for(unsigned tex = 0; tex < m_textures.size(); ++tex) {
		GLint attr = glGetUniformLocation(programId, m_textures[tex].name.c_str());

		GL_CHECK_ERR;

		if(attr >= 0)
			m_textures[tex].texture->use(attr, GL_TEXTURE0 + tex);
		// else
		// 	state.addWarning("Texture '" + m_textures[tex].name + "' cannot be mapped to an attribute location - not
		// used in any of the programs?");

		GL_CHECK_ERR;
	}

	return state;
}

std::size_t Uniforms::size() const {
	return m_uniforms.size() + m_textures.size();
}

bool Uniforms::empty() const {
	return m_uniforms.empty() && m_textures.empty();
}

std::string Uniforms::glslDeclaration() const {
	std::stringstream result;

	for(auto& u : m_uniforms)
		result << u->glslType << std::endl;

	for(auto& t : m_textures)
		result << t.glslType << std::endl;

	return result.str();
}

std::set<std::string> Uniforms::names() const {
	std::set<std::string> result;

	for(auto& u : m_uniforms)
		result.insert(u->name);

	for(auto& t : m_textures)
		result.insert(t.name);

	return result;
}

std::ostream& operator<<(std::ostream& out, const Uniforms& uniforms) {
	if(!uniforms.m_uniforms.empty()) {
		out << uniforms.m_uniforms.size() << " uniforms:" << std::endl;

		for(auto& i : uniforms.m_uniforms)
			out << "  " << i->glslType << std::endl;
	}

	if(!uniforms.m_textures.empty()) {
		out << uniforms.m_textures.size() << " textures:" << std::endl;

		for(auto& i : uniforms.m_textures)
			out << "  " << i.glslType << std::endl;
	}

	return out;
}

}  // namespace possumwood
