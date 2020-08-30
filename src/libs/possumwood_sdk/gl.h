#pragma once

#include <GL/glew.h>

#include <GL/gl.h>

#include <boost/noncopyable.hpp>
#include <string>

namespace possumwood {

void glCheckError(const std::string& file, unsigned line);

class ScopedEnable : public boost::noncopyable {
  public:
	ScopedEnable(unsigned id);
	~ScopedEnable();

  private:
	unsigned m_id;
};

}  // namespace possumwood

/// A simple macro that prints out any OpenGL errors on cout.
#define GL_CHECK_ERR possumwood::glCheckError(__FILE__, __LINE__);
