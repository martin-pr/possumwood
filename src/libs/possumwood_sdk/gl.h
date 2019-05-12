#pragma once

#include <string>

#include <boost/noncopyable.hpp>

namespace possumwood {

void glCheckError(const std::string& file, unsigned line);

class ScopedEnable : public boost::noncopyable {
	public:
		ScopedEnable(unsigned id);
		~ScopedEnable();

	private:
		unsigned m_id;
};

}

/// A simple macro that prints out any OpenGL errors on cout.
#define GL_CHECK_ERR possumwood::glCheckError(__FILE__, __LINE__);
