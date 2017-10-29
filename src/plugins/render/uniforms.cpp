#include "uniforms.h"

#include <OpenEXR/ImathMatrix.h>

#include "datatypes/uniforms.inl"

namespace possumwood {

void addViewportUniforms(possumwood::Uniforms& uniforms) {
	uniforms.addUniform<std::array<double, 16>>(
		"iProjection",
		possumwood::Uniforms::kPerDraw,
		[]() {
			std::array<double, 16> projection;
			glGetDoublev(GL_PROJECTION_MATRIX, projection.data());

			return projection;
		}
	);

	uniforms.addUniform<std::array<double, 16>>(
		"iModelView",
		possumwood::Uniforms::kPerDraw,
		[]() {
			std::array<double, 16> modelview;
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview.data());

			return modelview;
		}
	);

	uniforms.addUniform<std::array<double, 16>>(
		"iModelViewNormal",
		possumwood::Uniforms::kPerDraw,
		[]() {
			std::array<double, 16> modelview;
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview.data());

			// normal matrix has to be inverted and transposed
			Imath::M44d mv;
			for(unsigned a=0;a<16;++a)
				mv[a/4][a%4] = modelview[a];

			mv = mv.inverse().transposed();

			for(unsigned a=0;a<16;++a)
				modelview[a] = mv[a/4][a%4];

			return modelview;
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
