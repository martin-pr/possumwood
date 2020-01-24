#pragma once

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMatrix.h>

namespace lightfields {

class Pattern {
	public:
		Pattern(double lensPitch, double pixelPitch, double rotation, Imath::V2d scaleFactor, Imath::V3d sensorOffset, Imath::V2i sensorResolution);
		Pattern(double lensPitch, Imath::M33d tr, Imath::V2i sensorResolution);
		~Pattern();

		Imath::V4d sample(const Imath::V2i& pixelPos) const;
		const Imath::V2i& sensorResolution() const;

	private:
		Pattern(const Pattern&) = delete;
		Pattern& operator = (const Pattern&) = delete;

		Imath::V2i m_sensorResolution;
		double m_lensPitch;

		Imath::M33d m_tr, m_trInv;
};

}
