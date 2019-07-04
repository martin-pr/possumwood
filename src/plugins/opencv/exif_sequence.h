#pragma once

#include <memory>
#include <vector>

#include <boost/iterator/indirect_iterator.hpp>

#include "exif.h"

namespace possumwood { namespace opencv {

class ExifSequence final {
	public:
		void add(const Exif& exif);

		bool empty() const;
		std::size_t size() const;

		typedef std::vector<Exif>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		bool operator == (const ExifSequence& f) const;
		bool operator != (const ExifSequence& f) const;

	private:
		std::vector<Exif> m_sequence;
};

std::ostream& operator << (std::ostream& out, const ExifSequence& f);

}

template<>
struct Traits<opencv::ExifSequence> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0, 1}};
	}
};

}
