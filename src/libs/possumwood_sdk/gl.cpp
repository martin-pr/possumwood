#include "gl.h"

#include <iostream>

#include <GL/glew.h>
#include <GL/gl.h>

namespace possumwood {

void glCheckError(const std::string& file, unsigned line) {
	GLenum err = glGetError();
	while(err != GL_NO_ERROR) {
		std::cout << "OpenGL error found in " << file << ":" << line << ": " << gluErrorString(err) << std::endl;

		err = glGetError();
	}
}

};

#define GL_CHECK_ERR possumwood::glCheckError(__FILE__, __LINE__);
