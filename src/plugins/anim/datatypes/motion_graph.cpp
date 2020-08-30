#include "motion_graph.h"

#include <cassert>
#include <random>
#include <set>

namespace anim {

MotionGraph::Snippet::Snippet(std::shared_ptr<const Animation> animation, std::size_t startFrame, std::size_t endFrame)
    : m_animation(animation), m_startFrame(startFrame), m_endFrame(endFrame) {
	assert(m_animation && !m_animation->empty());
	assert(endFrame >= startFrame && m_endFrame < m_animation->size() && m_startFrame < m_animation->size());
}

std::size_t MotionGraph::Snippet::size() const {
	return m_endFrame - m_startFrame + 1;
}

bool MotionGraph::Snippet::empty() const {
	return m_animation->empty();
}

std::size_t MotionGraph::Snippet::startFrame() const {
	return m_startFrame;
}

std::size_t MotionGraph::Snippet::endFrame() const {
	return m_endFrame;
}

void MotionGraph::Snippet::appendTo(anim::Animation& anim) const {
	// relative transformation, to allow "integrating" the overall root movement, while discounting any initial offset
	// of frames of the snippet. For this to not accumulate errors, the root transformation has to represent "idealised
	// trajectory". TODO: preprocessing of trajectories.
	anim::Transform relativeTr;
	if(!anim.empty())
		relativeTr = anim.back()[0].tr() * m_animation->frame(m_startFrame)[0].tr().inverse();

	// append the snippet's frames to the animation
	for(std::size_t f = m_startFrame + 1; f <= m_endFrame; ++f) {
		anim::Skeleton frame = m_animation->frame(f);
		frame[0].tr() = relativeTr * frame[0].tr();

		anim.addFrame(frame);
	}
}

void MotionGraph::Snippet::addTransition(TransitionType type, std::size_t snippetIndex) {
	m_transitions.push_back(Transition{type, snippetIndex});
}

const std::vector<MotionGraph::Snippet::Transition>& MotionGraph::Snippet::transitions() const {
	return m_transitions;
}

bool MotionGraph::Snippet::operator==(const Snippet& snippet) const {
	return m_animation == snippet.m_animation && m_startFrame == snippet.m_startFrame &&
	       m_endFrame == snippet.m_endFrame && m_transitions == snippet.m_transitions;
}

bool MotionGraph::Snippet::operator!=(const Snippet& snippet) const {
	return m_animation != snippet.m_animation || m_startFrame != snippet.m_startFrame ||
	       m_endFrame != snippet.m_endFrame || m_transitions != snippet.m_transitions;
}

bool MotionGraph::Snippet::Transition::operator==(const Transition& tr) const {
	return type == tr.type && snippetIndex == tr.snippetIndex;
}
bool MotionGraph::Snippet::Transition::operator!=(const Transition& tr) const {
	return type != tr.type || snippetIndex != tr.snippetIndex;
}

////////

MotionGraph::MotionGraph() : m_fps(24.0f) {
}

MotionGraph::MotionGraph(const anim::Animation& a, const std::vector<std::pair<std::size_t, std::size_t>>& transitions)
    : m_fps(a.fps()) {
	assert(!transitions.empty() && !a.empty());

	// first, cut the input based on in and out transition points
	std::set<std::size_t> cutPoints;
	cutPoints.insert(0);
	cutPoints.insert(a.size() - 1);

	for(auto& p : transitions) {
		assert(p.first < a.size());
		assert(p.second < a.size());

		cutPoints.insert(p.first);
		cutPoints.insert(p.second);
	}

	// start_frame -> snippetId for original animation snippets
	std::map<std::size_t, std::size_t> startIndex;
	// end_frame -> snippetId for original animation snippets
	std::map<std::size_t, std::size_t> endIndex;

	// create a snippet for each cutPoint and add self-transitions
	if(!cutPoints.empty()) {
		std::shared_ptr<const anim::Animation> anim(new anim::Animation(a));

		std::set<std::size_t>::const_iterator it1 = cutPoints.begin();
		std::set<std::size_t>::const_iterator it2 = cutPoints.begin();
		++it2;

		while(it2 != cutPoints.end()) {
			startIndex.insert(std::make_pair(*it1, m_snippets.size()));
			endIndex.insert(std::make_pair(*it2, m_snippets.size()));

			m_snippets.push_back(Snippet(anim, *it1, *it2));

			++it1;
			++it2;

			// if not at the end of the animation, add a "identity" transition to reproduce the original motion
			if(it2 != cutPoints.end())
				m_snippets.back().addTransition(Snippet::TransitionType::kOriginalAnimation, m_snippets.size());
		}
	}

	// for each transition, add a transition point between snippets
	// TODO: need generated transition blends, at some point
	for(auto& tr : transitions) {
		// there should not be any transitions on the first frame of the original animation
		if(tr.first > 0) {
			// get the "transition from" snippet
			auto from = endIndex.lower_bound(tr.first);
			assert(m_snippets[from->second].endFrame() == tr.first);

			// get the "transition to" snippet
			auto to = startIndex.lower_bound(tr.second);
			assert(m_snippets[to->second].startFrame() == tr.second);

			// and add the transition record to the "from" snippet
			m_snippets[from->second].addTransition(Snippet::TransitionType::kTransition, to->second);
		}
	}
}

bool MotionGraph::empty() const {
	return m_snippets.empty();
}

bool MotionGraph::operator==(const MotionGraph& mgraph) const {
	return m_snippets == mgraph.m_snippets;
}

bool MotionGraph::operator!=(const MotionGraph& mgraph) const {
	return m_snippets != mgraph.m_snippets;
}

namespace {

/// just a simple helper to derive probability from a transition
class ProbabilityDecider {
  public:
	ProbabilityDecider(float transitionProbability) : m_transitionProbability(transitionProbability) {
	}

	float operator()(const MotionGraph::Snippet::Transition& tr) const {
		if(tr.type == MotionGraph::Snippet::TransitionType::kOriginalAnimation)
			return 1.0f;
		else
			return m_transitionProbability;
	}

  private:
	float m_transitionProbability;
};

}  // namespace

anim::Animation MotionGraph::randomWalk(std::size_t targetFrameCount, float transitionProbability,
                                        std::size_t seed) const {
	// a simple random number generator setup
	std::mt19937 gen(seed);
	std::uniform_real_distribution<> dis(0.0f, 1.0f);

	// target animation
	anim::Animation result(m_fps);

	// a helper instance of "decider" - returns probability of transitions based on type
	const ProbabilityDecider probability(transitionProbability);

	// synthesize animation, starting with the "initial" snippet
	auto current = m_snippets.begin();
	while(result.size() < targetFrameCount) {
		current->appendTo(result);

		float totalProbability = 0.0f;
		for(auto& tr : current->transitions())
			totalProbability += probability(tr);

		if(totalProbability == 0.0f)
			break;

		else {
			const float currentValue = dis(gen) * totalProbability;
			auto it = current->transitions().begin();

			float sumProbability = probability(*it);
			while(sumProbability < currentValue) {
				++it;
				assert(it != current->transitions().end());

				sumProbability += probability(*it);
			}

			current = m_snippets.begin() + it->snippetIndex;
		}
	}

	return result;
}

std::ostream& operator<<(std::ostream& out, const MotionGraph& mgraph) {
	out << "(motion graph with " << mgraph.m_snippets.size() << " frames)";

	return out;
}

}  // namespace anim
