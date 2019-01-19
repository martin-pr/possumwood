#pragma once

#include <ImathMatrix.h>

namespace possumwood {

class ViewportState {
	public:
		ViewportState();

		void perspective(float fovyInDegrees, float znear, float zfar);
		void lookAt(const Imath::V3f& eyePosition, const Imath::V3f& lookAt, const Imath::V3f& upVector = Imath::V3f(0,1,0));
		void resize(unsigned width, unsigned height);

		unsigned width() const;
		unsigned height() const;

		const Imath::M44f projection() const;
		const Imath::M44f modelview() const;

	private:
		unsigned m_width, m_height;
		float m_fovyInDegrees, m_znear, m_zfar;
		Imath::V3f m_eyePosition, m_lookAt, m_upVector;
};

}
