#pragma once

#include <ImathMatrix.h>

namespace possumwood {

class ViewportState {
	public:
		ViewportState();

		void perspective(float fovyInDegrees, float znear, float zfar);
		void lookAt(const Imath::V3f& eyePosition, const Imath::V3f& lookAt, const Imath::V3f& upVector = Imath::V3f(0,1,0));
		void resize(std::size_t width, std::size_t height);

		std::size_t width() const;
		std::size_t height() const;

		const Imath::V3f& eyePosition() const;
		const Imath::V3f& target() const;
		const Imath::V3f& upVector() const;

		float fowInDegrees() const;
		float znear() const;
		float zfar();

		const Imath::M44f projection() const;
		const Imath::M44f modelview() const;

	private:
		std::size_t m_width, m_height;
		float m_fovyInDegrees, m_znear, m_zfar;
		Imath::V3f m_eyePosition, m_lookAt, m_upVector;
};

}
