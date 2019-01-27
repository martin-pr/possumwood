#pragma once

#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

/// WHY?
#ifndef GLAPI
#define GLAPI extern
#endif

#include <possumwood_sdk/viewport_state.h>

#include "action.h"

class Stack;
class GLFWwindow;

class RenderContext {
	public:
		RenderContext(const possumwood::ViewportState& viewport);
		~RenderContext();

		void run(Stack& stack);

		Action render(const possumwood::ViewportState& viewport, std::function<void(std::vector<GLubyte>&)> callback);

	private:
		RenderContext(const RenderContext&) = delete;
		RenderContext& operator = (const RenderContext&) = delete;

		GLFWwindow* m_window;
};
