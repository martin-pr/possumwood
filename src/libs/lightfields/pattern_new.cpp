#include "pattern_new.h"

#include <OpenEXR/ImathMatrix.h>

namespace lightfields {

NewPattern::NewPattern(double lensPitch, double pixelPitch, double rotation,
	Imath::V2d scaleFactor, Imath::V3d sensorOffset, Imath::V2i sensorResolution) : Pattern(sensorResolution),
	m_lensPitch(lensPitch), m_pixelPitch(pixelPitch), m_rotation(rotation),
	m_scaleFactor(scaleFactor), m_sensorOffset(sensorOffset)
{
}

Imath::V4d NewPattern::sample(const Imath::V2i& pixelPos) const {
	Imath::V4d result;

	const double cs = cos(m_rotation);
	const double sn = sin(m_rotation);

	Imath::M33d scale, pattern, rotate, transform;
	scale.makeIdentity();
	pattern.makeIdentity();
	transform.makeIdentity();

	scale[0][0] = m_pixelPitch / m_lensPitch / m_scaleFactor[0];
	scale[1][1] = m_pixelPitch / m_lensPitch / m_scaleFactor[1];

	rotate[0][0] = cs;
	rotate[0][1] = -sn;
	rotate[1][0] = sn;
	rotate[1][1] = cs;

	transform[2][0] = m_sensorOffset[0] / m_pixelPitch;
	transform[2][1] = m_sensorOffset[1] / m_pixelPitch;

	pattern[1][0] = -0.5 / sqrt(3.0/4.0);
	pattern[1][1] = 1.0 / sqrt(3.0/4.0);

	Imath::V3d pos(pixelPos[0], pixelPos[1], 1.0);

	pos = pos * rotate * transform * scale * pattern;
	pos = pos / pos[2];

	result[0] = 0.0;
	result[1] = 0.0;

	result[2] = pos[0] - round(pos[0]);
	result[3] = pos[1] - round(pos[1]);

	return result;
}

}
