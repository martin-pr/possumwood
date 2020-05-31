#include "texture.h"

#include <cassert>
#include <stdexcept>

namespace possumwood {

namespace {

template<typename T>
struct TextureTraits;

template<>
struct TextureTraits<const unsigned char*> {
	static constexpr GLint GLType() { return GL_UNSIGNED_BYTE; };
};

template<>
struct TextureTraits<const float*> {
	static constexpr GLint GLType() { return GL_FLOAT; };
};

template<typename T_PTR>
std::string makeTexture(GLuint id, T_PTR data, std::size_t width, std::size_t height, const Texture::Format& format) {
	std::string error;

	int original_alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &original_alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, format.row_byte_align);

	glBindTexture(GL_TEXTURE_2D, id);

	// this assumes all image data are stored as a flat 8-bit per channel array!
	switch(format.channel_order) {
		case Texture::kRGB: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, TextureTraits<T_PTR>::GLType(), data); break;
		case Texture::kBGR: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, TextureTraits<T_PTR>::GLType(), data); break;
		case Texture::kGray: glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, TextureTraits<T_PTR>::GLType(), data); break;
		default:
			assert(false && "Unsupported channel order");
			error = "Unsupported channel order for OpenGL texture storage.";
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, format.interpolation);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, format.interpolation);

	glBindTexture(GL_TEXTURE_2D, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, original_alignment);

	return error;
}

}

Texture::Texture(const unsigned char* data, std::size_t width, std::size_t height, const Format& format) : m_id(0) {
	glGenTextures(1, &m_id);
	const std::string err = makeTexture(m_id, data, width, height, format);

	if(!err.empty()) {
		glDeleteTextures(1, &m_id);
		throw std::runtime_error(err);
	}
}

Texture::Texture(const float* data, std::size_t width, std::size_t height, const Format& format) : m_id(0) {
	glGenTextures(1, &m_id);
	const std::string err = makeTexture(m_id, data, width, height, format);

	if(!err.empty()) {
		glDeleteTextures(1, &m_id);
		throw std::runtime_error(err);
	}
}

Texture::~Texture() {
	if(m_id != 0)
		glDeleteTextures(1, &m_id);
}

GLuint Texture::id() const {
	return m_id;
}

void Texture::use(GLint attribLocation, GLenum textureUnit) const {
	if(m_id != 0) {
		glUniform1i(attribLocation, textureUnit-GL_TEXTURE0);
		glActiveTexture(textureUnit);
		glBindTexture(GL_TEXTURE_2D, m_id);
	}
}

}
