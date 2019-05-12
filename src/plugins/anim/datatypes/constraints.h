#pragma once

#include <map>
#include <vector>
#include <limits>
#include <cassert>

#include <boost/range/iterator_range.hpp>

#include <actions/traits.h>

#include "transform.h"

namespace anim {

class Animation;

/// TODO: Interface of this class is just a read-only draft, and needs to be finalized.
class Constraints {
	public:
		Constraints() = default;
		Constraints(const anim::Animation& a);

		Constraints(const Constraints& c);
		const Constraints& operator = (const Constraints& c);

		class Channel;

		class Frame {
			public:
				const anim::Transform& tr() const;

			private:
				Frame(const anim::Transform& tr);

				anim::Transform m_tr;

			friend class Channel;
			friend class Constraints;
		};

		class Constraint {
			public:
				const anim::Transform& origin() const;
				std::size_t startFrame() const;
				std::size_t endFrame() const;

				// typedef boost::iterator_range<std::vector<Frame>::const_iterator> FrameRange;
				// typedef FrameRange::const_iterator const_iterator;
				// const_iterator begin() const;
				// const_iterator end() const;

				bool operator == (const Constraint& c) const;
				bool operator != (const Constraint& c) const;

			private:
				Constraint(const anim::Transform& origin, std::size_t start, std::size_t end);

				anim::Transform m_origin;
				std::size_t m_startFrame, m_endFrame;

			friend class Channel;
		};

		class Frames {
			public:
				const Frame& operator[](std::size_t index) const;

				typedef std::vector<Frame>::const_iterator const_iterator;
				const_iterator begin() const;
				const_iterator end() const;

				bool empty() const;
				std::size_t size() const;

			private:
				Frames();

				std::vector<Frame> m_frames;

			friend class Channel;
			friend class Constraints;
		};

		class Channel {
			public:
				typedef std::vector<Constraint>::const_iterator const_iterator;
				const_iterator begin() const;
				const_iterator end() const;

				std::size_t size() const;

				const Frames& frames() const;

				bool operator == (const Channel& c) const;
				bool operator != (const Channel& c) const;

			private:
				Channel(Constraints* parent);

				/// Adds a new constraint to the channel. All constraints should be non-overlapping.
				void addConstraint(std::size_t startFrame, std::size_t endFrame, const anim::Transform& origin);

				std::vector<Constraint> m_values;
				Constraints* m_parent;

				Frames m_frames;

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
