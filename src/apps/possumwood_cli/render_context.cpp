#include "render_context.h"

#include <stdexcept>
#include <string>
#include <chrono>
#include <thread>

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

void draw(const possumwood::ViewportState& viewport) {
	// setup the viewport basics
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glViewport(0,0,viewport.width, viewport.height);

	glClearColor(0, 1, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	possumwood::App::instance().draw(viewport, [](const dependency_graph::NodeBase& node) {
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

std::function<void()> s_idleFn;

void idleFn() {
	if(s_idleFn) {
		s_idleFn();
		s_idleFn = std::function<void()>();
	}

	else
		glutPostRedisplay();
}

}

Action RenderContext::render(const possumwood::ViewportState& viewport, std::function<void(std::vector<GLubyte>&)> callback) {
	return Action([viewport, callback]() {
		std::vector<Action> actions;

		std::shared_ptr<std::vector<GLubyte>> data(new std::vector<GLubyte>(viewport.width * viewport.height * 3));

		// initialisation
		actions.push_back(Action([data, viewport]() -> std::vector<Action> {
			ensureGLUTInitialised();

			s_idleFn = [viewport]() {
				glutReshapeWindow(viewport.width, viewport.height);
			};

			return std::vector<Action>();
		}));

		// do nothing for one iteration
		actions.push_back(Action([]() {
			return std::vector<Action>();
		}));

		// rendering
		actions.push_back(Action([viewport]() {
			GL_CHECK_ERR

			draw(viewport);

			GL_CHECK_ERR

			return std::vector<Action>();
		}));

		// reading back the buffers
		actions.push_back(Action([data, callback, viewport] {
			GL_CHECK_ERR

			// glReadBuffer(GL_BACK);

			GL_CHECK_ERR

			glReadPixels(0, 0, viewport.width, viewport.height, GL_RGB, GL_UNSIGNED_BYTE, &(*data)[0]);

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
	using namespace std::chrono_literals;

	if(!s_stack->isFinished()) {
		s_stack->step();

		glutPostRedisplay();
	}

	else
		glutLeaveMainLoop();
}

void reshapeFunc(int w, int h) {
	glViewport(0,0,w,h);
}

}

void RenderContext::run(Stack& stack) {
	s_stack = &stack;

	glutDisplayFunc(step);
	// glutIdleFunc(glutPostRedisplay);
	glutIdleFunc(idleFn);
	glutReshapeFunc(reshapeFunc);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();

	s_glutInitialised = false;

	s_stack = nullptr;
}
