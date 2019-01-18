#include "render_context.h"

#include <stdexcept>
#include <string>

#include <GL/freeglut.h>

#include <possumwood_sdk/gl.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/metadata.h>

namespace {

static bool s_glutInitialised = false;

void ensureGLUTInitialised() {
	if(!s_glutInitialised) {
		int count = 0;
		glutInit(&count, nullptr);

		s_glutInitialised = true;
	}
}

void ensureGLEWInitialised() {
	static bool s_initialised = false;

	if(!s_initialised) {
		auto glewErr = glewInit();
		if(glewErr != GLEW_OK)
			throw std::runtime_error(std::string("Error initialising GLEW - ") + (const char*)(glewGetErrorString(glewErr)));

		s_initialised = true;
	}
}

}

RenderContext::RenderContext(const possumwood::ViewportState& viewport) {
	ensureGLUTInitialised();

	glutInitWindowSize(viewport.width, viewport.height);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitContextVersion (4, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);

	m_windowId = glutCreateWindow("offscreen-ish");

	ensureGLEWInitialised();
}

RenderContext::~RenderContext() {
	// silly GLUT - https://sourceforge.net/p/freeglut/mailman/freeglut-developer/thread/BANLkTin7n06HnopO5qSiUgUBrN3skAWDyA@mail.gmail.com/
	ensureGLUTInitialised();

	glutDestroyWindow(m_windowId);
}

namespace {

possumwood::ViewportState s_viewportState;

void draw() {
	// setup the viewport basics
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	possumwood::App::instance().draw(s_viewportState, [](const dependency_graph::NodeBase& node) {
		boost::optional<possumwood::Drawable&> drawable = possumwood::Metadata::getDrawable(node);
		if(drawable && drawable->drawState().errored()) {
			std::stringstream ss;
			for(auto& e : drawable->drawState())
				ss << e.second << std::endl;

			throw std::runtime_error(ss.str());
		}

	});

	glFlush();
}

///

GLubyte* s_dataPtr;
bool s_firstRun;

void readFrame() {
	if(s_firstRun) {
		glutPostRedisplay();

		s_firstRun = false;
	}

	else {
		glReadBuffer(GL_BACK);

		glReadPixels(0, 0, s_viewportState.width, s_viewportState.height, GL_RGB, GL_UNSIGNED_BYTE, s_dataPtr);

		glutLeaveMainLoop();
	}
}

}

std::vector<GLubyte> RenderContext::render(const possumwood::ViewportState& viewport) {
	ensureGLUTInitialised();

	std::vector<GLubyte> buffer(viewport.width * viewport.height * 3);

	s_viewportState = viewport;
	s_dataPtr = &buffer[0];
	s_firstRun = true;

	glutDisplayFunc(draw);
	glutIdleFunc(readFrame);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();

	s_glutInitialised = false;

	s_dataPtr = nullptr;

	return buffer;
}
