#pragma once

#include <string>

namespace possumwood {

void glCheckError(const std::string& file, unsigned line);

}

/// A simple macro that prints out any OpenGL errors on cout.
#define GL_CHECK_ERR possumwood::glCheckError(__FILE__, __LINE__);
