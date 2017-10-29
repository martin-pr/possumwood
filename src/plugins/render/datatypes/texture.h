#pragma once

#include <boost/noncopyable.hpp>

#include <GL/glew.h>
#include <GL/gl.h>

#include "pixmap.h"

namespace possumwood {

class Texture : public boost::noncopyable {
	public:
		Texture(const QPixmap& pixmap);
		~Texture();

		GLuint id() const;

		void use(GLint attribLocation, GLenum textureUnit) const;

	private:
		GLuint m_id;
};

}
