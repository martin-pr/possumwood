#pragma once

#include <memory>
#include <iostream>
#include <vector>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "lightfield_pattern.h"

namespace possumwood { namespace opencv {

/// Creates 2D samples for lightfield refocusing (i.e., sampling a 2D plane in 4D space with constant U and V).
class LightfieldSamples {
	public:
		enum Color {
			kRed = 2,
			kGreen = 1,
			kBlue = 0
		};

		struct Sample {
			Imath::V2i source; ///< Integer position of the source pixel in source image
			Imath::V2f target; ///< Target position in the target image (normalized between 0..1!)
			Color color; ///< Target colour channel, based on the Bayer pattern
		};

		LightfieldSamples();
		LightfieldSamples(const ::lightfields::Pattern& pattern, float uvOffset, float uvThreshold, float xy_scale);
		~LightfieldSamples();

		using const_iterator = std::vector<Sample>::const_iterator;

		const_iterator begin(std::size_t row = 0) const;
		const_iterator end(std::size_t row = std::numeric_limits<std::size_t>::max()) const;

		std::size_t size() const;
		bool empty() const;

		bool operator == (const LightfieldSamples& f) const;
		bool operator != (const LightfieldSamples& f) const;

	private:
		struct Pimpl;
		std::shared_ptr<const Pimpl> m_pimpl;
};

std::ostream& operator << (std::ostream& out, const LightfieldSamples& f);

}

template<>
struct Traits<opencv::LightfieldSamples> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.5, 0, 0}};
	}
};

}
