#pragma once

#include <OpenEXR/ImathVec.h>

namespace lightfields {

class Pattern {
	public:
		Pattern(const Imath::V2i& resolution);
		virtual ~Pattern() = 0;

		virtual Imath::V4d sample(const Imath::V2i& pixelPos) const = 0;
		const Imath::V2i& sensorResolution() const;

	private:
		Pattern(const Pattern&) = delete;
		Pattern& operator = (const Pattern&) = delete;

		Imath::V2i m_sensorResolution;
};

}
