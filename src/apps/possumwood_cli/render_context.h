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

class RenderContext {
	public:
		RenderContext(const possumwood::ViewportState& viewport);
		~RenderContext();

		std::vector<GLubyte> render(const possumwood::ViewportState& viewport);

	private:
		RenderContext(const RenderContext&) = delete;
		RenderContext& operator = (const RenderContext&) = delete;

		int m_windowId;
};
