#pragma once

#include "pattern.h"

namespace lightfields {

class LytroPattern : public Pattern {
	public:
		LytroPattern(double lensPitch, double pixelPitch, double rotation,
			Imath::V2d scaleFactor, Imath::V3d sensorOffset, Imath::V2i sensorResolution);

		virtual Imath::V4d sample(const Imath::V2i& pixelPos) const override;

	private:
		double m_lensPitch, m_pixelPitch, m_rotation;
		Imath::V2d m_scaleFactor;
		Imath::V3d m_sensorOffset;
};

}
