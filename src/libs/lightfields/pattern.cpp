#include "pattern.h"

#include <OpenEXR/ImathMatrix.h>

namespace lightfields {

Pattern::Pattern() : m_lensPitch(1.0), m_pixelPitch(1.0), m_rotation(0.0), m_scaleFactor(1.0, 1.0), m_sensorResolution(100, 100) {
}

Pattern::Pattern(double lensPitch, double pixelPitch, double rotation,
	Imath::V2d scaleFactor, Imath::V3d sensorOffset, Imath::V2i sensorResolution) :
	m_lensPitch(lensPitch), m_pixelPitch(pixelPitch), m_rotation(rotation),
	m_scaleFactor(scaleFactor), m_sensorOffset(sensorOffset), m_sensorResolution(sensorResolution)
{
}

bool Pattern::operator == (const Pattern& f) const {
	return m_lensPitch == f.m_lensPitch &&
		m_pixelPitch == f.m_pixelPitch &&
		m_rotation == f.m_rotation &&
		m_scaleFactor == f.m_scaleFactor &&
		m_sensorOffset == f.m_sensorOffset &&
		m_sensorResolution == f.m_sensorResolution;
}

bool Pattern::operator != (const Pattern& f) const {
	return m_lensPitch != f.m_lensPitch ||
		m_pixelPitch != f.m_pixelPitch ||
		m_rotation != f.m_rotation ||
		m_scaleFactor != f.m_scaleFactor ||
		m_sensorOffset != f.m_sensorOffset ||
		m_sensorResolution != f.m_sensorResolution;
}

Imath::V4d Pattern::sample(const Imath::V2i& pixelPos) const {
	Imath::V4d result;

	const double cs = cos(m_rotation);
	const double sn = sin(m_rotation);

	Imath::M44d transform;
	transform.makeIdentity();

	transform[0][0] = 1.0/m_scaleFactor[0] * cs;
	transform[0][1] = 1.0/m_scaleFactor[1] * -sn;
	transform[1][0] = 1.0/m_scaleFactor[0] * sn;
	transform[1][1] = 1.0/m_scaleFactor[1] * cs;

	// "projection" - additional
	// transform[2][3] = -1;
	// pos[2] = m_sensorOffset[2];

	Imath::V4d pos;
	pos[0] = (double)pixelPos[0] * m_pixelPitch + m_sensorOffset[0];
	pos[1] = (double)pixelPos[1] * m_pixelPitch + m_sensorOffset[1];
	pos[3] = 1.0;

	// transform and normalize - we are now in a "normalized" set of coordinates - straightened, with scale applied, the grid is now axis-aligned
	pos = pos * transform;
	pos /= pos.w;


	Imath::V2d vect(
		pos[0] / m_lensPitch,
		pos[1] / m_lensPitch / sqrt(3.0/4.0)
	);

	if(((int)round(vect[1] + 100.0)) % 2 == 1)
		vect[0] += 0.5;

	result[2] = (vect[0] + 100.0 - round(vect[0] + 100.0));
	result[3] = (vect[1] + 100.0 - round(vect[1] + 100.0));

	// "centered" version - all pixels of a lens are centered on that lens
	result[0] = (double)pixelPos[0] - result[2] * m_lensPitch / m_pixelPitch * m_scaleFactor[0];
	result[1] = (double)pixelPos[1] - result[3] * m_lensPitch * sqrt(3.0/4.0) / m_pixelPitch * m_scaleFactor[1];

	// "original" version - pixel coordinates are the same as on the sensor
	// result[0] = (double)pixelPos[0];
	// result[1] = (double)pixelPos[1];

	result[2] *= 2.0;
	result[3] *= 2.0;

	return result;
}

const Imath::V2i& Pattern::sensorResolution() const {
	return m_sensorResolution;
}

std::ostream& operator << (std::ostream& out, const Pattern& f) {
	out << "(lightfields pattern)";
	return out;
}

}
