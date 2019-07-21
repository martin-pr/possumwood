#pragma once

#include <memory>
#include <iostream>

#include <boost/iterator/iterator_facade.hpp>

#include <actions/traits.h>

#include "lightfield_pattern.h"

namespace possumwood { namespace opencv {

/// Creates 2D samples for lightfield refocusing (i.e., sampling a 2D plane in 4D space with constant U and V).
/// Implements forward iterators - to facilitate parallel computation, it allows to grab offset iterators.
class LightfieldSamples {
	public:
		enum Color {
			kRed = 2,
			kGreen = 1,
			kBlue = 0
		};

		struct Sample {
			cv::Vec2i source; ///< Integer position of the source pixel in source image
			cv::Vec2f target; ///< Target position in the target image (normalized between 0..1!)
			Color color; ///< Target colour channel, based on the Bayer pattern
		};

		LightfieldSamples();
		LightfieldSamples(const LightfieldPattern& pattern, float uvOffset, float uvThreshold, float xy_scale);

		class const_iterator : public boost::iterator_facade<const_iterator, const Sample, boost::forward_traversal_tag> {
			public:
				const_iterator();

			private:
				const_iterator(const LightfieldSamples* parent, std::size_t index);

				void increment();
				bool equal(const const_iterator& other) const;
				const Sample& dereference() const;

				void incrementUntilValid();

				const LightfieldSamples* m_parent;
				std::size_t m_index;
				Sample m_value;

				friend class boost::iterator_core_access;
				friend class LightfieldSamples;
		};

		const_iterator begin(std::size_t row = 0) const;
		const_iterator end(std::size_t row = std::numeric_limits<std::size_t>::max()) const;

		bool operator == (const LightfieldSamples& f) const;
		bool operator != (const LightfieldSamples& f) const;

	private:
		LightfieldPattern m_pattern;
		double m_uvOffset, m_uvThreshold, m_xyScale;
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
