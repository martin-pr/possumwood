#include "vbo.h"

#include <cassert>

namespace possumwood {

VBOBase::VBOBase() {
	glGenBuffers(1, &m_id);
	assert(m_id != 0);
}

VBOBase::~VBOBase() {
	glDeleteBuffers(1, &m_id);
}

GLuint VBOBase::id() const {
	return m_id;
}

}
