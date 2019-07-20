#include "lightfield_samples.h"

namespace possumwood { namespace opencv {

LightfieldSamples::LightfieldSamples() : m_uvOffset(0.0f), m_uvThreshold(0.0f) {
}

LightfieldSamples::LightfieldSamples(const LightfieldPattern& pattern, float uvOffset, float uvThreshold) : m_pattern(pattern), m_uvOffset(uvOffset), m_uvThreshold(uvThreshold) {
}

LightfieldSamples::const_iterator::const_iterator() : m_parent(nullptr), m_index(0) {
}

LightfieldSamples::const_iterator::const_iterator(const LightfieldSamples* parent, std::size_t index) : m_parent(parent), m_index(index) {
	incrementUntilValid();
}

void LightfieldSamples::const_iterator::incrementUntilValid() {
	while(m_index < std::size_t(m_parent->m_pattern.sensorResolution()[0] * m_parent->m_pattern.sensorResolution()[1])) {
		const auto& size = m_parent->m_pattern.sensorResolution();

		const int x = m_index % size[0];
		const int y = m_index / size[0];

		const cv::Vec4f coords = m_parent->m_pattern.sample(cv::Vec2i(x,y));

		const double uv_magnitude_2 = coords[2]*coords[2] + coords[3]*coords[3];
		if(uv_magnitude_2 <= m_parent->m_uvThreshold*m_parent->m_uvThreshold) {
			m_value.source = cv::Vec2i(x, y);
			m_value.target = cv::Vec2f(
				(coords[0] + coords[2]*m_parent->m_uvOffset) / (float)size[0],
				(coords[1] + coords[3]*m_parent->m_uvOffset) / (float)size[1]

				// (coords[0]) / (float)size[0],
				// (coords[1]) / (float)size[1]
			);
			m_value.color = possumwood::opencv::LightfieldSamples::Color((x%2) + (y%2)); // hardcoded bayer pattern, for now

			if(m_value.target[0] >= 0.0f && m_value.target[1] >= 0.0f && m_value.target[0] < 1.0f && m_value.target[1] < 1.0f)
				return;
		}

		++m_index;
	}
}

void LightfieldSamples::const_iterator::increment() {
	assert(m_parent != nullptr);

	++m_index;
	incrementUntilValid();
}

bool LightfieldSamples::const_iterator::equal(const const_iterator& other) const {
	return m_parent == other.m_parent && m_index == other.m_index;
}

const LightfieldSamples::Sample& LightfieldSamples::const_iterator::dereference() const {
	return m_value;
}

bool LightfieldSamples::operator == (const LightfieldSamples& f) const {
	return m_pattern == f.m_pattern && m_uvOffset == f.m_uvOffset && m_uvThreshold == f.m_uvThreshold;
}
bool LightfieldSamples::operator != (const LightfieldSamples& f) const {
	return m_pattern != f.m_pattern || m_uvOffset != f.m_uvOffset || m_uvThreshold != f.m_uvThreshold;
}

LightfieldSamples::const_iterator LightfieldSamples::begin(std::size_t row) const {
	return const_iterator(this, m_pattern.sensorResolution()[0] * row);
}

LightfieldSamples::const_iterator LightfieldSamples::end(std::size_t row) const {
	if(row >= std::size_t(m_pattern.sensorResolution()[1]))
		return const_iterator(this, m_pattern.sensorResolution()[0] * m_pattern.sensorResolution()[1]);

	return const_iterator(this, m_pattern.sensorResolution()[0] * (row+1));
}

std::ostream& operator << (std::ostream& out, const LightfieldSamples& f) {
	out << "(lightfield samples)";

	return out;
}

} }
