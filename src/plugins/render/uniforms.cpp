#include "uniforms.h"

#include <OpenEXR/ImathMatrix.h>

#include "datatypes/uniforms.inl"

namespace possumwood {

void addViewportUniforms(possumwood::Uniforms& uniforms) {
	uniforms.addUniform<Imath::M44d>(
		"iProjection",
		possumwood::Uniforms::kPerDraw,
		[]() {
			Imath::M44d projection;
			glGetDoublev(GL_PROJECTION_MATRIX, projection.getValue());

			return projection;
		}
	);

	uniforms.addUniform<Imath::M44d>(
		"iModelView",
		possumwood::Uniforms::kPerDraw,
		[]() {
			Imath::M44d modelview;
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview.getValue());

			return modelview;
		}
	);

	uniforms.addUniform<Imath::M44d>(
		"iModelViewNormal",
		possumwood::Uniforms::kPerDraw,
		[]() {
			Imath::M44d mv;
			glGetDoublev(GL_MODELVIEW_MATRIX, mv.getValue());

			// normal matrix has to be inverted and transposed
			mv = mv.inverse().transposed();

			return mv;
		}
	);

	uniforms.addUniform<Imath::V2f>(
		"iResolution",
		possumwood::Uniforms::kPerDraw,
		[]() {
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			return Imath::V2f(viewport[2], viewport[3]);
		}
	);
}

}
