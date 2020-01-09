#include "pattern_new.h"

#include <cmath>

namespace lightfields {

NewPattern::NewPattern(double lensPitch, double pixelPitch, double rotation,
	Imath::V2d scaleFactor, Imath::V3d sensorOffset, Imath::V2i sensorResolution) : Pattern(sensorResolution)
{
	const double cs = cos(rotation);
	const double sn = sin(rotation);

	Imath::M33d scale, pattern, rotate, transform;
	scale.makeIdentity();
	pattern.makeIdentity();
	transform.makeIdentity();

	scale[0][0] = pixelPitch / lensPitch / scaleFactor[0];
	scale[1][1] = pixelPitch / lensPitch / scaleFactor[1];

	rotate[0][0] = cs;
	rotate[0][1] = -sn;
	rotate[1][0] = sn;
	rotate[1][1] = cs;

	transform[2][0] = sensorOffset[0] / pixelPitch;
	transform[2][1] = sensorOffset[1] / pixelPitch;

	pattern[1][0] = -0.5 / sqrt(3.0/4.0);
	pattern[1][1] = 1.0 / sqrt(3.0/4.0);

	m_tr = rotate * transform * scale * pattern;
	m_trInv = m_tr.inverse();
}

Imath::V4d NewPattern::sample(const Imath::V2i& pixelPos) const {
	Imath::V4d result;

	const Imath::V3d pos = Imath::V3d(pixelPos[0], pixelPos[1], 1.0) * m_tr;

	const Imath::V3d pos1 = Imath::V3d(floor(pos[0]), floor(pos[1]), 1.0);
	const Imath::V3d pos2 = Imath::V3d(floor(pos[0]), ceil(pos[1]), 1.0);
	const Imath::V3d pos3 = Imath::V3d(ceil(pos[0]), floor(pos[1]), 1.0);
	const Imath::V3d pos4 = Imath::V3d(ceil(pos[0]), ceil(pos[1]), 1.0);

	const Imath::V3d lens1 = pos1 * m_trInv;
	const Imath::V3d lens2 = pos2 * m_trInv;
	const Imath::V3d lens3 = pos3 * m_trInv;
	const Imath::V3d lens4 = pos4 * m_trInv;

	const float dist1 = std::pow(pixelPos[0] - lens1[0], 2) + std::pow(pixelPos[1] - lens1[1], 2);
	const float dist2 = std::pow(pixelPos[0] - lens2[0], 2) + std::pow(pixelPos[1] - lens2[1], 2);
	const float dist3 = std::pow(pixelPos[0] - lens3[0], 2) + std::pow(pixelPos[1] - lens3[1], 2);
	const float dist4 = std::pow(pixelPos[0] - lens4[0], 2) + std::pow(pixelPos[1] - lens4[1], 2);

	Imath::V3d lens = lens1;
	Imath::V3d pos0 = pos1;
	if(dist2 < dist1 && dist2 < dist3 && dist2 < dist4) {
		lens = lens2;
		pos0 = pos2;
	}
	else if(dist3 < dist1 && dist3 < dist2 && dist3 < dist4) {
		lens = lens3;
		pos0 = pos3;
	}
	else if(dist4 < dist1 && dist4 < dist2 && dist4 < dist3) {
		lens = lens4;
		pos0 = pos4;
	}

	// std::cout << dist1 << "  " << dist2 << "  " << dist3 << "  " << dist4 << std::endl;

	result[0] = lens[0];
	result[1] = lens[1];

	result[2] = pixelPos[0] - lens[0];
	result[3] = pixelPos[1] - lens[1];

	return result;
}

}
