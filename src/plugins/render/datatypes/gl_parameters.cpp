#include "gl_parameters.h"

#include <GL/gl.h>

namespace possumwood {

GLParameters::GLParameters() : m_lineWidth(1.0f) {
}

void GLParameters::apply() const {
	glLineWidth(m_lineWidth);
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

}
