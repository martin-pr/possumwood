#include "pattern.h"

#include <cmath>

#include <OpenEXR/ImathMatrix.h>

#include "metadata.h"
#include "lenslet_graph.h"

namespace lightfields {

Pattern::Pattern() : m_sensorResolution(0,0), m_lensPitch(0) {
}

Pattern::Pattern(float lensPitch, float pixelPitch, float rotation,
	Imath::V2d scaleFactor, Imath::V3d sensorOffset, Imath::V2i sensorResolution) : m_sensorResolution(sensorResolution), m_lensPitch(lensPitch/pixelPitch)
{
	// put together lens transformation matrix
	Imath::M33d scale, pattern, rotate, transform;
	scale.makeIdentity();
	pattern.makeIdentity();
	transform.makeIdentity();

	scale[0][0] = pixelPitch / lensPitch / scaleFactor[0];
	scale[1][1] = pixelPitch / lensPitch / scaleFactor[1];

	const float cs = cos(rotation);
	const float sn = sin(rotation);

	rotate[0][0] = cs;
	rotate[0][1] = -sn;
	rotate[1][0] = sn;
	rotate[1][1] = cs;

	transform[2][0] = sensorOffset[0] / pixelPitch;
	transform[2][1] = sensorOffset[1] / pixelPitch;

	pattern[1][0] = -0.5 / sqrt(3.0/4.0);
	pattern[1][1] = 1.0 / sqrt(3.0/4.0);

	// we need both forward and backward projection
	Imath::M33d tmp = rotate * transform * scale * pattern;

	m_tr = Imath::M33f(tmp);
	m_trInv = Imath::M33f(tmp.inverse());
}

Pattern::Pattern(float lensPitch, const Imath::M33d& tr, Imath::V2i sensorResolution) : m_sensorResolution(sensorResolution), m_lensPitch(lensPitch), m_tr(tr) {
	m_trInv = Imath::M33f(tr.inverse());
}

Pattern::~Pattern() {
}

const Imath::V2i& Pattern::sensorResolution() const {
	return m_sensorResolution;
}

Pattern::Sample Pattern::sample(const Imath::V2i& pixelPos) const {
	Sample result;

	// convert the pixel position to lens space
	const Imath::V3f pos = Imath::V3f(pixelPos[0], pixelPos[1], 1.0) * m_tr;

	// surrounding lens position by rounding the pos in lens space
	const Imath::V3f pos1 = Imath::V3f(floor(pos[0]), floor(pos[1]), 1.0);
	const Imath::V3f pos2 = Imath::V3f(pos1[0], pos1[1] + 1.0, 1.0);
	const Imath::V3f pos3 = Imath::V3f(pos1[0] + 1.0, pos1[1], 1.0);
	const Imath::V3f pos4 = Imath::V3f(pos1[0] + 1.0, pos1[1] + 1.0, 1.0);

	// convert back from lens space to to pixel space
	const Imath::V3f lens1 = pos1 * m_trInv;
	const Imath::V3f lens2 = pos2 * m_trInv;
	const Imath::V3f lens3 = pos3 * m_trInv;
	const Imath::V3f lens4 = pos4 * m_trInv;

	// find the nearest lens center in pixel space
	const float dist1 = std::pow(pixelPos[0] - lens1[0], 2) + std::pow(pixelPos[1] - lens1[1], 2);
	const float dist2 = std::pow(pixelPos[0] - lens2[0], 2) + std::pow(pixelPos[1] - lens2[1], 2);
	const float dist3 = std::pow(pixelPos[0] - lens3[0], 2) + std::pow(pixelPos[1] - lens3[1], 2);
	const float dist4 = std::pow(pixelPos[0] - lens4[0], 2) + std::pow(pixelPos[1] - lens4[1], 2);

	Imath::V3f lens = lens1;
	Imath::V3f pos0 = pos1;
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

	result.lensCenter[0] = lens[0];
	result.lensCenter[1] = lens[1];

	result.offset[0] = (pixelPos[0] - lens[0]) / m_lensPitch * 2.0;
	result.offset[1] = (pixelPos[1] - lens[1]) / m_lensPitch * 2.0;

	return result;
}

void Pattern::scale(float factor) {
	Imath::M33d tmp = Imath::M33d(m_tr);

	tmp =
		Imath::M33d(
			1, 0, 0,
			0, 1, 0,
			-m_sensorResolution[0]/2, -m_sensorResolution[1]/2, 1
		)
		*
		Imath::M33d(
			factor, 0, 0,
			0, factor, 0,
			0, 0, 1
		)
		*
		Imath::M33d(
			1, 0, 0,
			0, 1, 0,
			m_sensorResolution[0]/2, m_sensorResolution[1]/2, 1
		)
		*
		tmp;

	m_tr = Imath::M33f(tmp);
	m_trInv = Imath::M33f(tmp.inverse());
}

bool Pattern::operator == (const Pattern& p) const {
	return m_sensorResolution == p.m_sensorResolution && m_lensPitch == p.m_lensPitch && m_tr == p.m_tr;
}

bool Pattern::operator != (const Pattern& p) const {
	return m_sensorResolution != p.m_sensorResolution || m_lensPitch != p.m_lensPitch || m_tr != p.m_tr;
}

std::ostream& operator << (std::ostream& out, const Pattern& p) {
	out << "Pattern:" << std::endl;
	out << "  sensor " << p.sensorResolution() << std::endl;
	out << "  lens pitch " << p.m_lensPitch << std::endl;
	out << "  transform" << std::endl << p.m_tr << std::endl;

	return out;
}

/////

Pattern Pattern::fromMetadata(const Metadata& meta) {
	return lightfields::Pattern(
		meta.metadata()["devices"]["mla"]["lensPitch"].asDouble(),
		meta.metadata()["devices"]["sensor"]["pixelPitch"].asDouble(),
		meta.metadata()["devices"]["mla"]["rotation"].asDouble(),
		Imath::V2f(
			meta.metadata()["devices"]["mla"]["scaleFactor"]["x"].asDouble(),
			meta.metadata()["devices"]["mla"]["scaleFactor"]["y"].asDouble()
		),
		Imath::V3f(
			meta.metadata()["devices"]["mla"]["sensorOffset"]["x"].asDouble(),
			meta.metadata()["devices"]["mla"]["sensorOffset"]["y"].asDouble(),
			meta.metadata()["devices"]["mla"]["sensorOffset"]["z"].asDouble()
		),
		Imath::V2i(
			meta.metadata()["image"]["width"].asInt(),
			meta.metadata()["image"]["height"].asInt()
		)
	);
}

Pattern Pattern::fromFit(const LensletGraph& lg) {
	auto fitted = lg.fittedMatrix();

	Imath::M33d fittedMatrix(
		fitted(0, 0), fitted(0, 1), fitted(0, 2),
		fitted(1, 0), fitted(1, 1), fitted(1, 2),
		fitted(2, 0), fitted(2, 1), fitted(2, 2)
	);

	return lightfields::Pattern(lg.lensPitch(), fittedMatrix, lg.sensorResolution());
}

}
