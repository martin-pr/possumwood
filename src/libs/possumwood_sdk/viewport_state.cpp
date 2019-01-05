#include "viewport_state.h"

namespace possumwood {

ViewportState::ViewportState() : width(300), height(200) {
	perspective(46, 300, 200, 0.1, 100);
	lookAt(Imath::V3f(10, 10, 10), Imath::V3f(0, 0, 0));
}

void ViewportState::perspective(float fovyInDegrees, int w, int h, float znear, float zfar) {
	width = w;
	height = h;

	const float aspectRatio = (float)w / (float)h;

	const float f = 1.0 / tanf(fovyInDegrees * M_PI / 360.0);
	const float A = (zfar + znear) / (znear - zfar);
	const float B = 2.0 * zfar * znear / (znear - zfar);

	projection = Imath::M44f(f / aspectRatio, 0, 0, 0, 0, f, 0, 0, 0, 0, A, -1, 0, 0, B, 0);
}

void ViewportState::lookAt(const Imath::V3f& eyePosition, const Imath::V3f& lookAt,
                   const Imath::V3f& upVector) {
	Imath::V3f forward = lookAt - eyePosition;
	forward.normalize();

	Imath::V3f side = forward.cross(upVector);
	side.normalize();

	Imath::V3f up = side.cross(forward);
	up.normalize();

	Imath::M44f rotmat;
	rotmat.makeIdentity();

	rotmat[0][0] = side.x;
	rotmat[1][0] = side.y;
	rotmat[2][0] = side.z;

	rotmat[0][1] = up.x;
	rotmat[1][1] = up.y;
	rotmat[2][1] = up.z;

	rotmat[0][2] = -forward.x;
	rotmat[1][2] = -forward.y;
	rotmat[2][2] = -forward.z;

	Imath::M44f transmat;
	transmat.setTranslation(Imath::V3f(-eyePosition.x, -eyePosition.y, -eyePosition.z));

	modelview = transmat * rotmat;
}

}
