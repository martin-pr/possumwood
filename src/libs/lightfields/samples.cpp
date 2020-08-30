#include "samples.h"

#include <tbb/parallel_for.h>

#include <cassert>

namespace lightfields {

Samples::Samples() {
}

Samples::~Samples() {
}

void Samples::makeRowOffsets() {
	if(m_samples.empty())
		m_rowOffsets.clear();

	else {
		// initialise to maximum offset
		m_rowOffsets = std::vector<std::size_t>(m_size[1], m_samples.size());
		m_rowOffsets[0] = 0;

		// collect row start indices
		int current = 0;
		for(auto it = m_samples.begin(); it != m_samples.end(); ++it) {
			assert(it->source[1] >= 0 && it->source[1] < m_size[1]);
			assert(it->source[1] >= current);

			// propagate current value, to account for "empty" scanlines
			for(int r = current + 1; r < it->source[1]; ++r)
				m_rowOffsets[r] = m_rowOffsets[r - 1];
			current = it->source[1];

			// and update the current starting index
			if(m_rowOffsets[it->source[1]] > (std::size_t)(it - m_samples.begin()))
				m_rowOffsets[it->source[1]] = it - m_samples.begin();
		}
	}
}

void Samples::offset(float uvOffset) {
	tbb::parallel_for(std::size_t(0), m_samples.size(), [&](std::size_t i) {
		auto& s = m_samples[i];
		s.xy += s.uv * uvOffset;
	});
}

void Samples::threshold(float uvThreshold) {
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

void Samples::filterInvalid() {
	auto tgt = m_samples.begin();
	auto src = m_samples.begin();

	const float widthf = m_size[0];
	const float heightf = m_size[1];
	for(; src != m_samples.end(); ++src) {
		if(src->xy[0] >= 0.0f && src->xy[1] >= 0.0f && src->xy[0] < widthf && src->xy[1] < heightf) {
			*tgt = *src;
			++tgt;
		}
	}
	m_samples.resize(tgt - m_samples.begin());

	makeRowOffsets();
}

void Samples::scale(float xy_scale) {
	const Imath::V2f center(m_size[0] / 2, m_size[1] / 2);

	tbb::parallel_for(std::size_t(0), m_samples.size(), [&](std::size_t i) {
		auto& sample = m_samples[i];
		sample.xy = (sample.xy - center) * xy_scale + center;
	});
}

Samples::const_iterator Samples::begin(std::size_t row) const {
	auto it = m_samples.end();

	if(row < m_rowOffsets.size()) {
		assert(m_rowOffsets[row] <= m_samples.size());
		it = m_samples.begin() + m_rowOffsets[row];
	}
	assert(it <= m_samples.end());

	return it;
}

Samples::const_iterator Samples::end(std::size_t row) const {
	row = std::min(row, std::numeric_limits<std::size_t>::max() - 1);
	return begin(row + 1);
}

std::size_t Samples::size() const {
	return m_samples.size();
}

bool Samples::empty() const {
	return m_samples.empty();
}

const Imath::V2i Samples::sensorSize() const {
	return m_size;
}

/////////

Samples Samples::fromPattern(const Pattern& pattern) {
	Samples result;

	result.m_size = pattern.sensorResolution();

	result.m_samples.resize(result.m_size[0] * result.m_size[1]);

	// assemble the samples
	tbb::parallel_for(std::size_t(0), (std::size_t)result.m_size[0], [&](std::size_t y) {
		for(std::size_t x = 0; x < (std::size_t)result.m_size[1]; ++x) {
			Sample& sample = result.m_samples[y * result.m_size[0] + x];

			const Pattern::Sample coords = pattern.sample(Imath::V2i(x, y));

			// input image pixel position, integer in pixels
			sample.source = Imath::V2i(x, y);
			// and UV coordinates, -1..1
			sample.uv = coords.offset;

			// target pixel position, normalized (0..1) - adding the UV offset to handle displacement
			sample.xy = coords.lensCenter;

			// hardcoded bayer pattern, for now
			sample.color = Color((x % 2) + (y % 2));
		}
	});

	result.makeRowOffsets();

	return result;
}

/////////

std::ostream& operator<<(std::ostream& out, const Samples& f) {
	out << "(lightfield samples - " << f.size() << " samples)";

	return out;
}

}  // namespace lightfields
