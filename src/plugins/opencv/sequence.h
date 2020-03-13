#pragma once

#include <memory>
#include <vector>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/filesystem/path.hpp>

#include "frame.h"

namespace possumwood { namespace opencv {

class Sequence final {
	public:
		Sequence(std::size_t size = 0);

		Sequence clone() const;

		void add(const cv::Mat& frame);

		bool isValid() const;

		bool empty() const;
		std::size_t size() const;

		int type() const;
		int rows() const;
		int cols() const;

		Frame& operator[](std::size_t index);
		const Frame& operator[](std::size_t index) const;

		typedef std::vector<Frame>::iterator iterator;
		iterator begin();
		iterator end();

		typedef std::vector<Frame>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		const Frame& front() const;
		const Frame& back() const;

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
