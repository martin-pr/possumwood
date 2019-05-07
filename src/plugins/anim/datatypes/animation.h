#pragma once

#include <vector>

#include <actions/traits.h>

#include "skeleton.h"

namespace anim {

/// keyframed animation
/// Explicit setter to allow for error/consistency checking
class Animation {
	public:
		Animation(float fps = 24.0f);

		void addFrame(const Skeleton& f);
		void setFrame(const Skeleton& f, std::size_t index);

		const Skeleton& frame(std::size_t index) const;

		bool empty() const;
		std::size_t size() const;

		float fps() const;
		void setFps(float fps);

		typedef std::vector<Skeleton>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		const Skeleton& front() const;
		const Skeleton& back() const;

		bool operator == (const Animation& anim) const;
		bool operator != (const Animation& anim) const;

	private:
		std::vector<Skeleton> m_frames;
		float m_fps;
};

std::ostream& operator <<(std::ostream& out, const Animation& anim);

}

namespace possumwood {

template<>
struct Traits<anim::Animation> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 1.0, 0}};
	}
};

}
