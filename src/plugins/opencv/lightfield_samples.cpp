#include "lightfield_samples.h"

#include <tbb/parallel_for.h>

namespace possumwood { namespace opencv {

LightfieldSamples::LightfieldSamples() {
}

LightfieldSamples::LightfieldSamples(const lightfields::Pattern& pattern) {
	m_size = pattern.sensorResolution();

	m_samples.resize(m_size[0] * m_size[1]);

	// assemble the samples
	tbb::parallel_for(std::size_t(0), (std::size_t)m_size[0], [&](std::size_t y) {
		for(std::size_t x=0; x<(std::size_t)m_size[1]; ++x) {
			Sample& sample = m_samples[y*m_size[0] + x];

			const lightfields::Pattern::Sample coords = pattern.sample(Imath::V2i(x,y));

			// input image pixel position, integer in pixels
			sample.source = Imath::V2i(x, y);
			// and UV coordinates, -1..1
			sample.uv = coords.offset;

			// target pixel position, normalized (0..1) - adding the UV offset to handle displacement
			sample.xy = coords.lensCenter;

			// hardcoded bayer pattern, for now
			sample.color = possumwood::opencv::LightfieldSamples::Color((x%2) + (y%2));
		}
	});

	makeRowOffsets();
}

LightfieldSamples::~LightfieldSamples() {
}

void LightfieldSamples::makeRowOffsets() {
	if(m_samples.empty())
		m_rowOffsets.clear();

	else {
		// initialise to maximum offset
		m_rowOffsets = std::move(std::vector<std::size_t>(m_size[1], m_samples.size()));
		m_rowOffsets[0] = 0;

		// collect row start indices
		int current = 1;
		for(auto it = m_samples.begin(); it != m_samples.end(); ++it) {
			assert(it->source[1] >= 0 && it->source[1] < m_size[1]);

			// propagate current value, to account for "empty" scanlines
			while(current < it->source[1]) {
				m_rowOffsets[current] = m_rowOffsets[current-1];
				++current;
			}
			current = std::max(it->source[1], current);

			// and update the current starting index
			if(m_rowOffsets[it->source[1]] > (std::size_t)(it - m_samples.begin()))
				m_rowOffsets[it->source[1]] = it - m_samples.begin();
		}

		for(std::size_t i=1;i<m_rowOffsets.size();++i) {
			if(not (m_rowOffsets[i-1] <= m_rowOffsets[i]))
				std::cout << "Row " << i << " " << m_rowOffsets[i-1] << "/" << m_rowOffsets[i] << std::endl;
		}
	}
}

void LightfieldSamples::offset(float uvOffset) {
	for(auto& s : m_samples)
		s.xy += s.uv * uvOffset;
}

void LightfieldSamples::threshold(float uvThreshold) {
	auto tgt = m_samples.begin();
	auto src = m_samples.begin();

	uvThreshold *= uvThreshold;
	for(; src != m_samples.end(); ++src) {
		if(src->uv.length2() < uvThreshold) {
			*tgt = *src;
			++tgt;
		}
	}
	m_samples.resize(tgt - m_samples.begin());

	makeRowOffsets();
}

void LightfieldSamples::filterInvalid() {
	auto tgt = m_samples.begin();
	auto src = m_samples.begin();

	const float widthf = m_size[0];
	const float heightf = m_size[1];
	for(; src != m_samples.end(); ++src) {
		if(floor(src->xy[0]) >= 0.0f && floor(src->xy[1]) >= 0.0f && floor(src->xy[0]) < widthf && floor(src->xy[1]) < heightf) {
			*tgt = *src;
			++tgt;
		}
	}
	m_samples.resize(tgt - m_samples.begin());

	makeRowOffsets();
}

void LightfieldSamples::scale(float xy_scale) {
	for(auto& sample : m_samples)
		sample.xy = (sample.xy - Imath::V2f(m_size[0]/2, m_size[1]/2)) * xy_scale + Imath::V2f(m_size[0]/2, m_size[1]/2);

	makeRowOffsets();
}

LightfieldSamples::const_iterator LightfieldSamples::begin(std::size_t row) const {
	auto it = m_samples.end();

	if(row < m_rowOffsets.size()) {
		assert(m_rowOffsets[row] <= m_samples.size());
		it = m_samples.begin() + m_rowOffsets[row];
	}
	assert(it <= m_samples.end());

	return it;
}

LightfieldSamples::const_iterator LightfieldSamples::end(std::size_t row) const {
	row = std::min(row, std::numeric_limits<std::size_t>::max()-1);
	return begin(row+1);
}

std::size_t LightfieldSamples::size() const {
	return m_samples.size();
}

bool LightfieldSamples::empty() const {
	return m_samples.empty();
}

const Imath::V2i LightfieldSamples::sensorSize() const {
	return m_size;
}

std::ostream& operator << (std::ostream& out, const LightfieldSamples& f) {
	out << "(lightfield samples - " << f.size() << " samples)";

	return out;
}

} }
