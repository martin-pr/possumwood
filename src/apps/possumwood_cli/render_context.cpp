#include "render_context.h"

#include <stdexcept>
#include <string>

#include <possumwood_sdk/gl.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/metadata.h>

namespace {

bool s_glewInitialised = false;

}

RenderContext::RenderContext() {
    static const int attribs[] = {
       OSMESA_FORMAT, OSMESA_RGBA,
       OSMESA_DEPTH_BITS, 32,
       OSMESA_STENCIL_BITS, 0,
       OSMESA_ACCUM_BITS, 0,
       OSMESA_PROFILE, OSMESA_CORE_PROFILE,
       OSMESA_CONTEXT_MAJOR_VERSION, 3,
       OSMESA_CONTEXT_MINOR_VERSION, 3,
       0 };

	m_ctx = OSMesaCreateContextAttribs(attribs, NULL);
	if(!m_ctx)
		throw std::runtime_error("Mesa context creation failed");
}

RenderContext::~RenderContext() {
	OSMesaDestroyContext(m_ctx);
}

std::vector<GLubyte> RenderContext::render(const possumwood::ViewportState& viewport) {
	std::vector<GLubyte> buffer(viewport.width * viewport.height * 4);
	if(!OSMesaMakeCurrent(m_ctx, &buffer[0], GL_UNSIGNED_BYTE, viewport.width, viewport.height))
		throw std::runtime_error("OSMesaMakeCurrent call failed");

	if(!s_glewInitialised) {
		auto glewErr = glewInit();
		if(glewErr != GLEW_OK)
			throw std::runtime_error(std::string("Error initialising GLEW - ") + (const char*)(glewGetErrorString(glewErr)));
		s_glewInitialised = true;
	}

	GL_CHECK_ERR;

	// setup the viewport basics
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

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

	return buffer;
}
