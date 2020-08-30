#include "gl.h"

#include <GL/gl.h>

#include <iostream>

namespace possumwood {

void glCheckError(const std::string& file, unsigned line) {
	GLenum err = glGetError();
	while(err != GL_NO_ERROR) {
		std::cout << "OpenGL error found in " << file << ":" << line << ": " << gluErrorString(err) << std::endl;

		err = glGetError();
	}
}

ScopedEnable::ScopedEnable(unsigned id) : m_id(id) {
	glEnable(id);
}

ScopedEnable::~ScopedEnable() {
	glDisable(m_id);
}

};  // namespace possumwood

#define GL_CHECK_ERR possumwood::glCheckError(__FILE__, __LINE__);
