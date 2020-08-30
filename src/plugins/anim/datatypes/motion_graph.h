#pragma once

#include <limits>

#include "animation.h"

namespace anim {

class MotionGraph {
  public:
	MotionGraph();
	MotionGraph(const Animation& a, const std::vector<std::pair<std::size_t, std::size_t>>& transitions);

	bool empty() const;

	anim::Animation randomWalk(std::size_t targetFrameCount, float transitionProbability = 0.1,
	                           std::size_t seed = 0) const;

	bool operator==(const MotionGraph& mgraph) const;
	bool operator!=(const MotionGraph& mgraph) const;

	class Snippet {
	  public:
		enum TransitionType { kOriginalAnimation = 0, kTransition = 1 };

		struct Transition {
			TransitionType type;
			std::size_t snippetIndex;

			bool operator==(const Transition& tr) const;
			bool operator!=(const Transition& tr) const;
		};

		Snippet(std::shared_ptr<const Animation> animation, std::size_t startFrame = 0,
		        std::size_t endFrame = std::numeric_limits<std::size_t>::infinity());

		std::size_t size() const;
		bool empty() const;

		std::size_t startFrame() const;
		std::size_t endFrame() const;

		void appendTo(anim::Animation& anim) const;
		void addTransition(TransitionType type, std::size_t snippetIndex);

		const std::vector<Transition>& transitions() const;

		bool operator==(const Snippet& snippet) const;
		bool operator!=(const Snippet& snippet) const;

	  private:
		std::shared_ptr<const Animation> m_animation;
		std::size_t m_startFrame, m_endFrame;

		std::vector<Transition> m_transitions;

		friend class MotionGraph;
	};

  protected:
  private:
	std::vector<Snippet> m_snippets;
	float m_fps;

	friend std::ostream& operator<<(std::ostream& out, const MotionGraph& mgraph);
};

std::ostream& operator<<(std::ostream& out, const MotionGraph& mgraph);

}  // namespace anim

namespace possumwood {

template <>
struct Traits<anim::MotionGraph> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.5, 1, 0}};
	}
};

}  // namespace possumwood
