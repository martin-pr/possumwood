#include <gl_debug.h>

#include <iostream>

#include <GL/glew.h>

namespace {

#ifndef NDEBUG
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
	        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity,
	        message);
}
#endif

}

GlDebug::GlDebug() {
#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
}

GlDebug::~GlDebug() {
#ifndef NDEBUG
	glDisable(GL_DEBUG_OUTPUT);
#endif
}
