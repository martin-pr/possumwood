#pragma once

#include <memory>
#include <iostream>
#include <vector>
#include <limits>

#include "pattern.h"

namespace lightfields {

/// Creates 2D samples for lightfield refocusing (i.e., sampling a 2D plane in 4D space with constant U and V).
class Samples {
	public:
		enum Color {
			kRed = 2,
			kGreen = 1,
			kBlue = 0
		};

		struct Sample {
			Imath::V2i source; ///< Integer position of the source pixel in source image
			Imath::V2f xy; ///< XY coordinates of the sample
			Imath::V2f uv; ///< UV coordinates of the sample
			Color color; ///< Target colour channel, based on the Bayer pattern
		};

		Samples();
		~Samples();

		Samples(const Samples&) = default;
		Samples& operator = (const Samples&) = default;

		Samples(Samples&&) = default;
		Samples& operator = (Samples&&) = default;

		using const_iterator = std::vector<Sample>::const_iterator;

		const_iterator begin(std::size_t row = 0) const;
		const_iterator end(std::size_t row = std::numeric_limits<std::size_t>::max()) const;

		std::size_t size() const;
		bool empty() const;

		const Imath::V2i sensorSize() const;

		void offset(float uvOffset);
		void threshold(float uvThreshold);
		void scale(float xy_scale);
		void filterInvalid();

		static Samples fromPattern(const Pattern& p);

	private:
		void makeRowOffsets();

		std::vector<Sample> m_samples;
		std::vector<std::size_t> m_rowOffsets;
		Imath::V2i m_size;
};

std::ostream& operator << (std::ostream& out, const Samples& f);

}

