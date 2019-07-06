#include "lightfield_pattern.h"

namespace possumwood { namespace opencv {

LightfieldPattern::LightfieldPattern() : m_lensPitch(0.0), m_pixelPitch(0.0), m_rotation(0.0) {
}

LightfieldPattern::LightfieldPattern(double lensPitch, double pixelPitch, double rotation, 
	cv::Vec2d scaleFactor, cv::Vec3d sensorOffset, cv::Vec2i sensorResolution) :
	m_lensPitch(lensPitch), m_pixelPitch(pixelPitch), m_rotation(rotation),
	m_scaleFactor(scaleFactor), m_sensorOffset(sensorOffset), m_sensorResolution(sensorResolution)
{
}

bool LightfieldPattern::operator == (const LightfieldPattern& f) const {
	return m_lensPitch == f.m_lensPitch && 
		m_pixelPitch == f.m_pixelPitch && 
		m_rotation == f.m_rotation &&
		m_scaleFactor == f.m_scaleFactor &&
		m_sensorOffset == f.m_sensorOffset &&
		m_sensorResolution == f.m_sensorResolution;
}

bool LightfieldPattern::operator != (const LightfieldPattern& f) const {
	return m_lensPitch != f.m_lensPitch || 
		m_pixelPitch != f.m_pixelPitch || 
		m_rotation != f.m_rotation ||
		m_scaleFactor != f.m_scaleFactor ||
		m_sensorOffset != f.m_sensorOffset ||
		m_sensorResolution != f.m_sensorResolution;
}

cv::Vec4f LightfieldPattern::sample(const cv::Vec2i& pixelPos) const {
	cv::Vec4f result;

	result[0] = (double)pixelPos[0];
	result[1] = (double)pixelPos[1];

	const double xDiff = m_lensPitch / m_pixelPitch * m_scaleFactor[0];
	const double yDiff = m_lensPitch / m_pixelPitch * sqrt(3.0/4.0) * m_scaleFactor[1];

	cv::Vec2d vect(
		(double)pixelPos[0] / m_scaleFactor[0], 
		(double)pixelPos[1] / m_scaleFactor[1]
	);

	const double cs = cos(m_rotation);
	const double sn = sin(m_rotation);
	
	vect = cv::Vec2d (
		(vect[0] * cs + vect[1] * sn),
		(-vect[0] * sn + vect[1] * cs)
	);

	vect[0] += m_sensorOffset[0] / m_pixelPitch;
	vect[1] += m_sensorOffset[1] / m_pixelPitch;

	result[3] = vect[1] / yDiff;

	if(((int)round(result[3])) % 2 == 0)
		result[2] = fmod(vect[0] / xDiff + 100.5, 1.0) - 0.5;
	else
		result[2] = fmod(vect[0] / xDiff + 100.0, 1.0) - 0.5;

	result[3] = fmod(result[3] + 100.5, 1.0) - 0.5;

	result[2] *= 2.0f;
	result[3] *= 2.0f;

	return result;
}

std::ostream& operator << (std::ostream& out, const LightfieldPattern& f) {
	out << "(lightfields pattern)";
	return out;
}

} }
