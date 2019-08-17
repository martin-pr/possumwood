#pragma once

#include <OpenEXR/ImathVec.h>

namespace lightfields {

class Pattern {
	public:
		Pattern();
		Pattern(double lensPitch, double pixelPitch, double rotation,
			Imath::V2d scaleFactor, Imath::V3d sensorOffset, Imath::V2i sensorResolution);

		Imath::V4d sample(const Imath::V2i& pixelPos) const;
		const Imath::V2i& sensorResolution() const;

		bool operator == (const Pattern& f) const;
		bool operator != (const Pattern& f) const;

	private:
		double m_lensPitch, m_pixelPitch, m_rotation;
		Imath::V2d m_scaleFactor;
		Imath::V3d m_sensorOffset;
		Imath::V2i m_sensorResolution;
};

std::ostream& operator << (std::ostream& out, const Pattern& f);

}
