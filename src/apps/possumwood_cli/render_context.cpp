#include "render_context.h"

#include <stdexcept>
#include <string>

#include <GL/freeglut.h>

#include <possumwood_sdk/gl.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/metadata.h>

#include "stack.h"

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

}

Action RenderContext::render(const possumwood::ViewportState& viewport, std::function<void(std::vector<GLubyte>&)> callback) {
	return Action([viewport, callback]() {
		std::vector<Action> actions;

		std::shared_ptr<std::vector<GLubyte>> data(new std::vector<GLubyte>(viewport.width * viewport.height * 3));

		// initialisation
		actions.push_back(Action([data, viewport]() -> std::vector<Action> {
			ensureGLUTInitialised();

			s_viewportState = viewport;

			return std::vector<Action>();
		}));

		// rendering
		actions.push_back(Action([]() {
			GL_CHECK_ERR

			draw();

			GL_CHECK_ERR

			return std::vector<Action>();
		}));

		// reading back the buffers
		actions.push_back(Action([data, callback] {
			GL_CHECK_ERR

			// glReadBuffer(GL_BACK);

			GL_CHECK_ERR

			glReadPixels(0, 0, s_viewportState.width, s_viewportState.height, GL_RGB, GL_UNSIGNED_BYTE, &(*data)[0]);

			GL_CHECK_ERR

			callback(*data);

			GL_CHECK_ERR

			return std::vector<Action>();
		}));

		return actions;
	});
}

namespace {

Stack* s_stack;

void step() {
	if(!s_stack->isFinished())
		s_stack->step();

	else
		glutLeaveMainLoop();
}

}

void RenderContext::run(Stack& stack) {
	s_stack = &stack;

	// glutDisplayFunc(draw);
	// glutIdleFunc(readFrame);
	glutDisplayFunc(step);
	glutIdleFunc(glutPostRedisplay);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();

	s_glutInitialised = false;

	s_stack = nullptr;
}
