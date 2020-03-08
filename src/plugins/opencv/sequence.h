#pragma once

#include <memory>
#include <vector>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/filesystem/path.hpp>

#include "frame.h"

namespace possumwood { namespace opencv {

class Sequence final {
	public:
		void add(const cv::Mat& frame);

		bool empty() const;
		std::size_t size() const;

		typedef std::vector<Frame>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		bool operator == (const Sequence& f) const;
		bool operator != (const Sequence& f) const;

	private:
		std::vector<Frame> m_sequence;
};

std::ostream& operator << (std::ostream& out, const Sequence& f);

}

template<>
struct Traits<opencv::Sequence> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0, 0}};
	}
};

}
