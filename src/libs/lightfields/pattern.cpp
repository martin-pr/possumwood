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

	Imath::M33d transform;
	transform.makeIdentity();

	const double scale_x = m_pixelPitch / m_lensPitch / m_scaleFactor[0];
	const double scale_y = m_pixelPitch / m_lensPitch / m_scaleFactor[1] / sqrt(3.0/4.0);

	transform[0][0] = scale_x * cs;
	transform[0][1] = scale_y * -sn;
	transform[1][0] = scale_x * sn;
	transform[1][1] = scale_y * cs;
	transform[2][0] = scale_x * m_sensorOffset[0] / m_pixelPitch;
	transform[2][1] = scale_y * m_sensorOffset[1] / m_pixelPitch;

	Imath::V3d pos(pixelPos[0], pixelPos[1], 1.0);

	pos = pos * transform;


	Imath::V2f bottom(pos[0] - floor(pos[0] + 0.5), pos[1] - floor(pos[1] + 0.5));
	Imath::V2f top(bottom[0], bottom[1]>0.0 ? -1.0+bottom[1] : 1.0+bottom[1]);

	if(((int)floor(pos[1] + 0.5)) & 1) {
		bottom[0] += 0.5;
		if(bottom[0] > 0.5)
			bottom[0] -= 1.0;
	}
	else {
		top[0] += 0.5;
		if(top[0] > 0.5)
			top[0] -= 1.0;
	}

	top[1] *= sqrt(3.0/4.0);
	bottom[1] *= sqrt(3.0/4.0);

	if(bottom.length2() < top.length2()) {
		result[2] = bottom[0];
		result[3] = bottom[1];
	}
	else {
		result[2] = top[0];
		result[3] = top[1];
	}

	result[0] = (double)pixelPos[0] - result[2] / scale_x * sqrt(3.0/4.0);
	result[1] = (double)pixelPos[1] - result[3] / scale_y;

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
