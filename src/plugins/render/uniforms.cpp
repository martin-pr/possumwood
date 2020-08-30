#include "uniforms.h"

#include <OpenEXR/ImathMatrix.h>

#include "datatypes/uniforms.inl"

namespace possumwood {

void addViewportUniforms(possumwood::Uniforms& uniforms) {
	uniforms.addUniform<Imath::M44f>(
	    "iProjection", 1, possumwood::Uniforms::kPerDraw,
	    [](Imath::M44f* data, std::size_t size, const possumwood::ViewportState& vs) { *data = vs.projection(); });

	uniforms.addUniform<Imath::M44f>(
	    "iModelView", 1, possumwood::Uniforms::kPerDraw,
	    [](Imath::M44f* data, std::size_t size, const possumwood::ViewportState& vs) { *data = vs.modelview(); });

	uniforms.addUniform<Imath::M44f>("iModelViewNormal", 1, possumwood::Uniforms::kPerDraw,
	                                 [](Imath::M44f* data, std::size_t size, const possumwood::ViewportState& vs) {
		                                 Imath::M44f mv = vs.modelview();

		                                 // normal matrix has to be inverted and transposed
		                                 mv = mv.inverse().transposed();

		                                 *data = mv;
	                                 });

	uniforms.addUniform<Imath::V2f>("iResolution", 1, possumwood::Uniforms::kPerDraw,
	                                [](Imath::V2f* data, std::size_t size, const possumwood::ViewportState& vs) {
		                                *data = Imath::V2f(vs.width(), vs.height());
	                                });
}

}  // namespace possumwood
