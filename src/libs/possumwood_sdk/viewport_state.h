#pragma once

#include <ImathMatrix.h>

namespace possumwood {

struct ViewportState {
	ViewportState();

	void perspective(float fovyInDegrees, float znear, float zfar);
	void lookAt(const Imath::V3f& eyePosition, const Imath::V3f& lookAt, const Imath::V3f& upVector = Imath::V3f(0,1,0));

	unsigned width, height;
	Imath::M44f projection, modelview;
};

}
