#include "lightfield_samples.h"

namespace possumwood { namespace opencv {

struct LightfieldSamples::Pimpl {
	Pimpl() : size(0,0) {
	}

	std::vector<Sample> m_samples;
	std::vector<std::size_t> m_rowOffsets;
	Imath::V2i size;
};

LightfieldSamples::LightfieldSamples() : m_pimpl(new Pimpl()) {
}

LightfieldSamples::LightfieldSamples(const lightfields::Pattern& pattern, float uvOffset, float uvThreshold, float xy_scale) {
	std::unique_ptr<Pimpl> impl(new Pimpl());

	impl->size = pattern.sensorResolution();

	impl->m_samples.resize(impl->size[0] * impl->size[1]);
	std::vector<float> uvOffsets(impl->size[0] * impl->size[1], 0.0f);

	// assemble the samples
	for(std::size_t y=0; y<(std::size_t)impl->size[0]; ++y)
		for(std::size_t x=0; x<(std::size_t)impl->size[1]; ++x) {
			Sample& sample = impl->m_samples[y*impl->size[0] + x];

			const Imath::V4f coords = pattern.sample(Imath::V2i(x,y));

			// input image pixel position, integer in pixels
			sample.source = Imath::V2i(x, y);

			// target pixel position, normalized (0..1) - adding the UV offset to handle displacement
			sample.target = Imath::V2f(
				(coords[0] + coords[2]*uvOffset) / (float)(impl->size[0]),
				(coords[1] + coords[3]*uvOffset) / (float)(impl->size[1])

				// (coords[0]) / (float)size[0],
				// (coords[1]) / (float)size[1]
			);

			// compute detail scaling (usable to visualise details in the centre of the view, or crop edges of an image)
			sample.target = (sample.target - Imath::V2f(0.5f, 0.5f)) * xy_scale + Imath::V2f(0.5f, 0.5f);

			// hardcoded bayer pattern, for now
			sample.color = possumwood::opencv::LightfieldSamples::Color((x%2) + (y%2));

			const double uv_magnitude_2 = coords[2]*coords[2] + coords[3]*coords[3];
			uvOffsets[y*impl->size[0] + x] = uv_magnitude_2;
		}

	// filter out any samples outside the threshold
	{
		impl->m_rowOffsets.resize(impl->size[0], 0);

		auto it = impl->m_samples.begin();
		auto current = it;

		auto offs = uvOffsets.begin();

		int rowId = 0;

		while(it != impl->m_samples.end()) {
			// starts of each row
			while(rowId < it->source[1]) {
				++rowId;
				impl->m_rowOffsets[rowId] = current - impl->m_samples.begin();
			}

			// keep only samples within the requested XY region
			if(*offs < uvThreshold*uvThreshold && it->target[0] >= 0.0f && it->target[1] >= 0.0f && it->target[0] < 1.0f && it->target[1] < 1.0f) {
				*current = *it;
				++current;
			}

			++it;
			++offs;
		}

		++rowId;
		while(rowId < (int)impl->m_rowOffsets.size()) {
			impl->m_rowOffsets[rowId] = impl->m_samples.size();
			++rowId;
		}

		impl->m_samples.resize(current - impl->m_samples.begin());

		#ifndef NDEBUG

		// sanity checks
		{
			auto it = impl->m_rowOffsets.begin();
			assert(*it == 0);

			auto it2 = it++;
			while(it != impl->m_rowOffsets.end()) {
				assert(*it >= *it2);

				++it;
				++it2;
			}
		}

		#endif
	}

	m_pimpl = std::shared_ptr<const Pimpl>(impl.release());

	#ifndef NDEBUG

	for(auto it = begin(); it != end(); ++it) {
		assert(it->target[0] >= 0);
		assert(it->target[1] >= 0);
		assert(it->target[0] < 1.0f);
		assert(it->target[1] < 1.0f);
	}

	#endif
}

LightfieldSamples::~LightfieldSamples() {
}

bool LightfieldSamples::operator == (const LightfieldSamples& f) const {
	return m_pimpl == f.m_pimpl;
}
bool LightfieldSamples::operator != (const LightfieldSamples& f) const {
	return m_pimpl != f.m_pimpl;
}

LightfieldSamples::const_iterator LightfieldSamples::begin(std::size_t row) const {
	auto it = m_pimpl->m_samples.end();

	if(row < m_pimpl->m_rowOffsets.size())
		it = m_pimpl->m_samples.begin() + m_pimpl->m_rowOffsets[row];

	assert(it <= m_pimpl->m_samples.end());

	return it;
}

LightfieldSamples::const_iterator LightfieldSamples::end(std::size_t row) const {
	row = std::min(row, std::numeric_limits<std::size_t>::max()-1);
	return begin(row+1);
}

std::size_t LightfieldSamples::size() const {
	return m_pimpl->m_samples.size();
}

bool LightfieldSamples::empty() const {
	return m_pimpl->m_samples.empty();
}

std::ostream& operator << (std::ostream& out, const LightfieldSamples& f) {
	out << "(lightfield samples)";

	return out;
}

} }
