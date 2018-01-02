#include "gl_parameters.h"

#include <GL/gl.h>

namespace possumwood {

GLParameters::GLParameters() : m_lineWidth(1.0f) {
}

std::unique_ptr<GLParameters::ScopedState> GLParameters::apply() const {
	return std::unique_ptr<GLParameters::ScopedState>(new ScopedState(m_lineWidth));
}

void GLParameters::setLineWidth(float w) {
	m_lineWidth = w;
}

float GLParameters::lineWidth() const {
	return m_lineWidth;
}

bool GLParameters::operator == (const GLParameters& p) const {
	return m_lineWidth == p.m_lineWidth;
}

bool GLParameters::operator != (const GLParameters& p) const {
	return m_lineWidth != p.m_lineWidth;
}

///////

GLParameters::ScopedState::ScopedState(float lineWidth) {
	glGetFloatv(GL_LINE_WIDTH, &m_lineWidth);

	glLineWidth(lineWidth);
}

GLParameters::ScopedState::~ScopedState() {
	glLineWidth(m_lineWidth);
}

}
