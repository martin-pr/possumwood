#include "viewport_state.h"

namespace possumwood {

ViewportState::ViewportState() : m_width(300), m_height(200) {
	perspective(45, 0.1, 100);
	lookAt(Imath::V3f(10, 10, 10), Imath::V3f(0, 0, 0));
}

void ViewportState::perspective(float fovyInDegrees, float znear, float zfar) {
	m_fovyInDegrees = fovyInDegrees;
	m_znear = znear;
	m_zfar = zfar;
}

void ViewportState::lookAt(const Imath::V3f& eyePosition, const Imath::V3f& lookAt,
                   const Imath::V3f& upVector) {

	m_eyePosition = eyePosition;
	m_lookAt = lookAt;
	m_upVector = upVector;
}

void ViewportState::resize(unsigned width, unsigned height) {
	m_width = width;
	m_height = height;
}

unsigned ViewportState::width() const {
	return m_width;
}

unsigned ViewportState::height() const {
	return m_height;
}

const Imath::V3f& ViewportState::eyePosition() const {
	return m_eyePosition;
}

const Imath::V3f& ViewportState::target() const {
	return m_lookAt;
}

const Imath::V3f& ViewportState::upVector() const {
	return m_upVector;
}

const Imath::M44f ViewportState::projection() const {
	const float aspectRatio = (float)m_width / (float)m_height;

	const float f = 1.0 / tanf(m_fovyInDegrees * M_PI / 360.0);
	const float A = (m_zfar + m_znear) / (m_znear - m_zfar);
	const float B = 2.0 * m_zfar * m_znear / (m_znear - m_zfar);

	Imath::M44f projection = Imath::M44f(
		f / aspectRatio, 0, 0, 0,
		0, f, 0, 0,
		0, 0, A, -1,
		0, 0, B, 0
	);

	return projection;
}

const Imath::M44f ViewportState::modelview() const {
	Imath::V3f forward = m_lookAt - m_eyePosition;
	forward.normalize();

	Imath::V3f side = forward.cross(m_upVector);
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
	transmat.setTranslation(Imath::V3f(-m_eyePosition.x, -m_eyePosition.y, -m_eyePosition.z));

	Imath::M44f modelview = transmat * rotmat;
	return modelview;
}


}
