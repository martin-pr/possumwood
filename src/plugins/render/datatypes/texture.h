#pragma once

#include <boost/noncopyable.hpp>

#include <GL/glew.h>
#include <GL/gl.h>

namespace possumwood {

class Texture : public boost::noncopyable {
	public:
		Texture(const unsigned char* data, std::size_t width, std::size_t height);
		Texture(const float* data, std::size_t width, std::size_t height);
		~Texture();

		GLuint id() const;

		void use(GLint attribLocation, GLenum textureUnit) const;

	private:
		GLuint m_id;
};

}
