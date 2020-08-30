#include "texture.h"

#include <cassert>
#include <stdexcept>

namespace possumwood {

namespace {

template <typename T>
struct TextureTraits;

void applySingleTexture(GLint internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type,
                        const void* data) {
	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, type, data);
}

template <typename T>
void applyArrayTexture(GLint internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type,
                       const std::vector<const T*>& data) {
	for(std::size_t i = 0; i < data.size(); ++i)
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, format, type, data[i]);
}

template <>
struct TextureTraits<const unsigned char*> {
	static void init(GLint internalformat, GLsizei width, GLsizei height, const unsigned char* data){/* noop */};
	static constexpr GLint GLType() {
		return GL_UNSIGNED_BYTE;
	};
	static constexpr GLint BindType() {
		return GL_TEXTURE_2D;
	};
	static constexpr auto apply = applySingleTexture;
};

template <>
struct TextureTraits<const float*> {
	static void init(GLint internalformat, GLsizei width, GLsizei height, const float* data){/* noop */};
	static constexpr GLint GLType() {
		return GL_FLOAT;
	};
	static constexpr GLint BindType() {
		return GL_TEXTURE_2D;
	};
	static constexpr auto apply = applySingleTexture;
};

template <>
struct TextureTraits<std::vector<const unsigned char*>> {
	static void init(GLint internalformat, GLsizei width, GLsizei height,
	                 const std::vector<const unsigned char*>& data) {
		glTexStorage3D(GL_TEXTURE_2D_ARRAY,
		               1,  // no mipmaps, for now
		               internalformat, width, height, data.size());
	};
	static constexpr GLint GLType() {
		return GL_UNSIGNED_BYTE;
	};
	static constexpr GLint BindType() {
		return GL_TEXTURE_2D_ARRAY;
	};
	static constexpr auto apply = applyArrayTexture<unsigned char>;
};

template <typename T_PTR>
std::string makeTexture(GLuint id, T_PTR data, std::size_t width, std::size_t height, const Texture::Format& format) {
	std::string error;

	int original_alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &original_alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, format.row_byte_align);

	glBindTexture(TextureTraits<T_PTR>::BindType(), id);

	// this assumes all image data are stored as a flat 8-bit per channel array!
	switch(format.channel_order) {
		case Texture::kRGB:
			TextureTraits<T_PTR>::init(GL_RGB8, width, height, data);
			TextureTraits<T_PTR>::apply(GL_RGB, width, height, GL_RGB, TextureTraits<T_PTR>::GLType(), data);
			break;
		case Texture::kBGR:
			TextureTraits<T_PTR>::init(GL_RGB8, width, height, data);
			TextureTraits<T_PTR>::apply(GL_RGB, width, height, GL_BGR, TextureTraits<T_PTR>::GLType(), data);
			break;
		case Texture::kGray:
			TextureTraits<T_PTR>::init(GL_R8, width, height, data);
			TextureTraits<T_PTR>::apply(GL_R, width, height, GL_RED, TextureTraits<T_PTR>::GLType(), data);
			break;
		default:
			error = "Unsupported channel order for OpenGL texture storage.";
			break;
	}

	glTexParameteri(TextureTraits<T_PTR>::BindType(), GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(TextureTraits<T_PTR>::BindType(), GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(TextureTraits<T_PTR>::BindType(), GL_TEXTURE_MIN_FILTER, format.interpolation);
	glTexParameteri(TextureTraits<T_PTR>::BindType(), GL_TEXTURE_MAG_FILTER, format.interpolation);

	glBindTexture(TextureTraits<T_PTR>::BindType(), 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, original_alignment);

	return error;
}

}  // namespace

Texture::Texture(const unsigned char* data, std::size_t width, std::size_t height, const Format& format) : m_id(0) {
	glGenTextures(1, &m_id);
	const std::string err = makeTexture(m_id, data, width, height, format);

	m_bindType = GL_TEXTURE_2D;

	if(!err.empty()) {
		glDeleteTextures(1, &m_id);
		throw std::runtime_error(err);
	}
}

Texture::Texture(const float* data, std::size_t width, std::size_t height, const Format& format) : m_id(0) {
	glGenTextures(1, &m_id);
	const std::string err = makeTexture(m_id, data, width, height, format);

	m_bindType = GL_TEXTURE_2D;

	if(!err.empty()) {
		glDeleteTextures(1, &m_id);
		throw std::runtime_error(err);
	}
}

Texture::Texture(const std::vector<const unsigned char*>& data, std::size_t width, std::size_t height,
                 const Format& format) {
	glGenTextures(1, &m_id);

	const std::string err = makeTexture(m_id, data, width, height, format);

	m_bindType = GL_TEXTURE_2D_ARRAY;

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
		glUniform1i(attribLocation, textureUnit - GL_TEXTURE0);
		glActiveTexture(textureUnit);
		glBindTexture(m_bindType, m_id);
	}
}

}  // namespace possumwood
