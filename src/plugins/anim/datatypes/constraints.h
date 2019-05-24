#pragma once

#include <map>
#include <vector>
#include <limits>
#include <cassert>

#include <boost/range/iterator_range.hpp>

#include <actions/traits.h>

#include "constraints/channel.h"

namespace anim {

class Animation;

/// TODO: Interface of this class is just a read-only draft, and needs to be finalized.
class Constraints {
	public:
		Constraints() = default;
		Constraints(const anim::Animation& a);

		Constraints(const Constraints& c);
		const Constraints& operator = (const Constraints& c);

		typedef std::map<std::string, constraints::Channel>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		// detect constraints by world-space velocity thresholding.
		// TODO: move to a separate header file, to avoid mixing containers and algorithms?
		void addVelocityConstraint(const std::string& jointName, float velocityThreshold);

		bool empty() const;
		std::size_t size() const;

		bool operator == (const Constraints& c) const;
		bool operator != (const Constraints& c) const;

	private:
		// source animation - shared ptr, because it doesn't need to be copied or changed.
		// Should not be exposed in the public interface - implementation might change.
		std::shared_ptr<const anim::Animation> m_anim;

		// joint -> channel (set of non-overlapping constraints)
		std::map<std::string, constraints::Channel> m_channels;
};

std::ostream& operator <<(std::ostream& out, const Constraints& c);

}

namespace possumwood {

template<>
struct Traits<anim::Constraints> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1.0, 1.0, 0}};
	}
};

}
