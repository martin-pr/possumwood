#include "texture.h"

namespace possumwood {

Texture::Texture(const unsigned char* data, std::size_t width, std::size_t height, std::size_t row_byte_align) : m_id(0) {
	int original_alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &original_alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, row_byte_align);

	glGenTextures(1, &m_id);

	glBindTexture(GL_TEXTURE_2D, m_id);

	// this assumes all image data are stored as a flat 8-bit per channel array!
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, original_alignment);
}

Texture::Texture(const float* data, std::size_t width, std::size_t height, std::size_t row_byte_align) : m_id(0) {
	int original_alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &original_alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, row_byte_align);

	glGenTextures(1, &m_id);

	glBindTexture(GL_TEXTURE_2D, m_id);

	// this assumes all image data are stored as a flat float per channel array!
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, original_alignment);
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
