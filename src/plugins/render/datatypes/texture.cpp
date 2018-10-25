#include "texture.h"

namespace possumwood {

Texture::Texture(const QPixmap& pixmap) {
	QImage image = pixmap.toImage();

	image = image.convertToFormat(QImage::Format_RGBA8888);
	assert(image.format() == QImage::Format_RGBA8888);

	glGenTextures(1, &m_id);

	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
	             image.constBits());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
	glDeleteTextures(1, &m_id);
}

GLuint Texture::id() const {
	return m_id;
}

void Texture::use(GLint attribLocation, GLenum textureUnit) const {
	glUniform1i(attribLocation, textureUnit-GL_TEXTURE0);
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, m_id);
}

}
