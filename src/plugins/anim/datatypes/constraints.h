#pragma once

#include <map>
#include <vector>
#include <limits>
#include <cassert>

#include <actions/traits.h>

#include "transform.h"

namespace anim {

class Animation;

class Constraints {
	public:
		Constraints() = default;
		Constraints(const anim::Animation& a);

		class Channel;

		class Constraint {
			public:
				const anim::Transform& origin() const;
				std::size_t startFrame() const;
				std::size_t endFrame() const;

				bool operator == (const Constraint& c) const;
				bool operator != (const Constraint& c) const;

			private:
				Constraint(const anim::Transform& origin, std::size_t start, std::size_t end);

				anim::Transform m_origin;
				std::size_t m_startFrame, m_endFrame;

			friend class Channel;
		};

		class Channel {
			public:
				typedef std::vector<Constraint>::const_iterator const_iterator;
				const_iterator begin() const;
				const_iterator end() const;

				std::size_t size() const;

				bool operator == (const Channel& c) const;
				bool operator != (const Channel& c) const;

			private:
				/// Adds a new constraint to the channel. All constraints should be non-overlapping.
				void addConstraint(std::size_t startFrame, std::size_t endFrame, const anim::Transform& origin);

				std::vector<Constraint> m_values;

			friend class Constraints;
		};

		typedef std::map<std::string, Channel>::const_iterator const_iterator;
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
		std::map<std::string, Channel> m_channels;
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
