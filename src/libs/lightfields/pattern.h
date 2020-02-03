#pragma once

#include <iostream>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMatrix.h>

namespace lightfields {

class Pattern {
	public:
		Pattern();
		Pattern(double lensPitch, double pixelPitch, double rotation, Imath::V2d scaleFactor, Imath::V3d sensorOffset, Imath::V2i sensorResolution);
		Pattern(double lensPitch, Imath::M33d tr, Imath::V2i sensorResolution);
		~Pattern();

		void scale(float factor);

		struct Sample {
			Imath::V2f lensCenter; ///< center of the nearest lens
			Imath::V2f offset;     ///< normalized uv offset from the lens center (if inside a lens radius, should be inside a unit 2D circle (-1..1))
		};

		Sample sample(const Imath::V2i& pixelPos) const;
		const Imath::V2i& sensorResolution() const;

		bool operator == (const Pattern& p) const;
		bool operator != (const Pattern& p) const;

	private:
		Imath::V2i m_sensorResolution;
		double m_lensPitch;

		Imath::M33d m_tr, m_trInv;

	friend std::ostream& operator << (std::ostream& out, const Pattern& p);
};

std::ostream& operator << (std::ostream& out, const Pattern& p);

}
