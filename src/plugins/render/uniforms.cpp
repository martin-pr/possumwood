#include "uniforms.h"

#include <OpenEXR/ImathMatrix.h>

#include "datatypes/uniforms.inl"

namespace possumwood {

void addViewportUniforms(possumwood::Uniforms& uniforms) {
	uniforms.addUniform<Imath::M44d>(
		"iProjection",
		1,
		possumwood::Uniforms::kPerDraw,
		[](Imath::M44d* data, std::size_t size, const possumwood::Drawable::ViewportState& vs) {
			Imath::M44d projection;
			glGetDoublev(GL_PROJECTION_MATRIX, projection.getValue());

			*data = projection;
		}
	);

	uniforms.addUniform<Imath::M44d>(
		"iModelView",
		1,
		possumwood::Uniforms::kPerDraw,
		[](Imath::M44d* data, std::size_t size, const possumwood::Drawable::ViewportState& vs) {
			Imath::M44d modelview;
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview.getValue());

			*data = modelview;
		}
	);

	uniforms.addUniform<Imath::M44d>(
		"iModelViewNormal",
		1,
		possumwood::Uniforms::kPerDraw,
		[](Imath::M44d* data, std::size_t size, const possumwood::Drawable::ViewportState& vs) {
			Imath::M44d mv;
			glGetDoublev(GL_MODELVIEW_MATRIX, mv.getValue());

			// normal matrix has to be inverted and transposed
			mv = mv.inverse().transposed();

			*data = mv;
		}
	);

	uniforms.addUniform<Imath::V2f>(
		"iResolution",
		1,
		possumwood::Uniforms::kPerDraw,
		[](Imath::V2f* data, std::size_t size, const possumwood::Drawable::ViewportState& vs) {
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			*data = Imath::V2f(viewport[2], viewport[3]);
		}
	);
}

}
