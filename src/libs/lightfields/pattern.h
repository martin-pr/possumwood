#pragma once

#include <iostream>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMatrix.h>

namespace lightfields {

class Metadata;
class LensletGraph;

class Pattern {
	public:
		Pattern();
		~Pattern();

		/// adjusts the scale of the pattern (centered) - allows to compensate for inaccuracies in pattern detection, otherwise clearly visible
		/// on u-x or v-y EPI image as a black tilted line.
		void scale(float factor);

		struct Sample {
			Imath::V2f lensCenter; ///< center of the nearest lens
			Imath::V2f offset;     ///< normalized uv offset from the lens center (if inside a lens radius, should be inside a unit 2D circle (-1..1))
		};

		Sample sample(const Imath::V2i& pixelPos) const;
		const Imath::V2i& sensorResolution() const;

		bool operator == (const Pattern& p) const;
		bool operator != (const Pattern& p) const;

		/// creates a new pattern instance from Lytro metadata extracted from a raw file
		static Pattern fromMetadata(const Metadata& meta);
		/// creates a new pattern instance from an explicitly fitted pattern
		static Pattern fromFit(const LensletGraph& lg);

	private:
		Pattern(float lensPitch, float pixelPitch, float rotation, Imath::V2d scaleFactor, Imath::V3d sensorOffset, Imath::V2i sensorResolution);
		Pattern(float lensPitch, const Imath::M33d& tr, Imath::V2i sensorResolution);

		Imath::V2i m_sensorResolution;
		float m_lensPitch;

		Imath::M33f m_tr, m_trInv;

	friend std::ostream& operator << (std::ostream& out, const Pattern& p);
};

std::ostream& operator << (std::ostream& out, const Pattern& p);

}
