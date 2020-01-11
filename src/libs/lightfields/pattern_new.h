#pragma once

#include <OpenEXR/ImathMatrix.h>

#include "pattern.h"

namespace lightfields {

class NewPattern : public Pattern {
	public:
		NewPattern(double lensPitch, double pixelPitch, double rotation,
			Imath::V2d scaleFactor, Imath::V3d sensorOffset, Imath::V2i sensorResolution);

		virtual Imath::V4d sample(const Imath::V2i& pixelPos) const override;
		const Imath::V2i& sensorResolution() const;

	private:
		Imath::M33d m_tr, m_trInv;
};

}
