#include "vbo.h"

#include <cassert>

namespace possumwood {

VBOBase::VBOBase() : m_initialised(false) {
	glGenBuffers(1, &m_id);
	assert(m_id != 0);
}

VBOBase::~VBOBase() {
	glDeleteBuffers(1, &m_id);
}

GLuint VBOBase::id() const {
	return m_id;
}

bool VBOBase::isInitialised() const {
	return m_initialised;
}

void VBOBase::setInitialised(bool val) {
	m_initialised = val;
}

void VBOBase::use(GLint attribLocation) const {
	assert(attribLocation >= 0);

	glBindBuffer(GL_ARRAY_BUFFER, id());

	glEnableVertexAttribArray(attribLocation);
	glVertexAttribPointer(attribLocation, width(), type(), false, 0, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}
