#include "render_context.h"

#include <GLFW/glfw3.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/gl.h>
#include <possumwood_sdk/metadata.h>

#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>

#include "stack.h"

namespace {

void glfwErrorCallback(int error, const char* description) {
	std::cerr << "GLFW error: " << description << " (" << error << ")" << std::endl;
}

static bool s_glfwInitialised = false;

void ensureGLFWInitialised() {
	if(!s_glfwInitialised) {
		glfwInit();

		glfwSetErrorCallback(glfwErrorCallback);

		s_glfwInitialised = true;
	}
}

void ensureGLEWInitialised() {
	static bool s_initialised = false;

	if(!s_initialised) {
		auto glewErr = glewInit();
		if(glewErr != GLEW_OK)
			throw std::runtime_error(std::string("Error initialising GLEW - ") +
			                         (const char*)(glewGetErrorString(glewErr)));

		s_initialised = true;
	}
}

// Hacky version handling - it is difficult to figure out what version of OpenGL is supported
// without trying to create a context first, seeing it fail, and trying again with a lower number.
// This makes use of defines in glew.h instead.

std::pair<unsigned, unsigned> findGLVersion() {
#if GL_VERSION_6_0
	return std::make_pair(6, 0);
#elif GL_VERSION_4_7
	return std::make_pair(4, 7);
#elif GL_VERSION_4_6
	return std::make_pair(4, 6);
#elif GL_VERSION_4_5
	return std::make_pair(4, 5);
#elif GL_VERSION_4_4
	return std::make_pair(4, 4);
#elif GL_VERSION_4_3
	return std::make_pair(4, 3);
#elif GL_VERSION_4_2
	return std::make_pair(4, 2);
#elif GL_VERSION_4_1
	return std::make_pair(4, 1);
#elif GL_VERSION_4_0
	return std::make_pair(4, 0);
#endif
	return std::make_pair(0, 0);
}

}  // namespace

RenderContext::RenderContext(const possumwood::ViewportState& viewport) : m_window(nullptr) {
	ensureGLFWInitialised();

	std::pair<unsigned, unsigned> gl_ver = findGLVersion();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_ver.first);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_ver.second);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, false);

	m_window = glfwCreateWindow(viewport.width(), viewport.height(), "offscreen-ish", NULL, NULL);

	if(!m_window)
		throw std::runtime_error("GLFW window creation failed.");

	glfwMakeContextCurrent(m_window);

	std::cout << "OpenGL version supported by this platform is " << glGetString(GL_VERSION) << std::endl;

	ensureGLEWInitialised();
}

RenderContext::~RenderContext() {
	// silly GLUT -
	// https://sourceforge.net/p/freeglut/mailman/freeglut-developer/thread/BANLkTin7n06HnopO5qSiUgUBrN3skAWDyA@mail.gmail.com/
	ensureGLFWInitialised();

	glfwDestroyWindow(m_window);
}

namespace {

void draw(const possumwood::ViewportState& viewport) {
	// setup the viewport basics
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glViewport(0, 0, viewport.width(), viewport.height());

	glClearColor(0, 0, 0, 0);
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

}  // namespace

Action RenderContext::render(const possumwood::ViewportState& viewport,
                             std::function<void(std::vector<GLubyte>&)> callback) {
	return Action([this, viewport, callback]() {
		std::vector<Action> actions;

		std::shared_ptr<std::vector<GLubyte>> data(new std::vector<GLubyte>(viewport.width() * viewport.height() * 3));

		// initialisation
		actions.push_back(Action([this, data, viewport]() -> std::vector<Action> {
			ensureGLFWInitialised();

			glfwSetWindowSize(m_window, viewport.width(), viewport.height());

			return std::vector<Action>();
		}));

		// do nothing for one iteration
		actions.push_back(Action([]() { return std::vector<Action>(); }));

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

			glReadBuffer(GL_BACK);

			GL_CHECK_ERR

			glReadPixels(0, 0, viewport.width(), viewport.height(), GL_RGB, GL_UNSIGNED_BYTE, &(*data)[0]);

			GL_CHECK_ERR

			callback(*data);

			GL_CHECK_ERR

			return std::vector<Action>();
		}));

		return actions;
	});
}

void RenderContext::run(Stack& stack) {
	using namespace std::chrono_literals;

	while(!stack.isFinished()) {
		stack.step();

		glfwSwapBuffers(m_window);
		glfwPollEvents();

		// a little delay to allow GLFW to run its "idle" loop
		std::this_thread::sleep_for(10ms);
	}
}
