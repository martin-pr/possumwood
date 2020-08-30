#include "skinning.h"

#include <cassert>

namespace anim {

void Skinning::addWeight(std::size_t bone, float weight) {
	m_weights.push_back(Weight(bone, weight));
}

void Skinning::normalize() {
	float total = 0.0f;
	for(auto& w : m_weights)
		total += w.weight;

	if(total > 0.0f)
		for(auto& w : m_weights)
			w.weight /= total;
}

void Skinning::limitInfluenceCount(std::size_t count) {
	assert(count > 0);

	while(count < m_weights.size()) {
		float minVal = 0.0f;
		std::size_t minIndex = 0;

		for(std::size_t i = 0; i < m_weights.size(); ++i)
			if(minVal > m_weights[i].weight) {
				minVal = m_weights[i].weight;
				minIndex = i;
			}

		m_weights.erase(m_weights.begin() + minIndex);
	}

	normalize();
}

bool Skinning::empty() const {
	return m_weights.empty();
}

std::size_t Skinning::size() const {
	return m_weights.size();
}

Skinning::const_iterator Skinning::begin() const {
	return m_weights.begin();
}

Skinning::const_iterator Skinning::end() const {
	return m_weights.end();
}

Skinning::iterator Skinning::begin() {
	return m_weights.begin();
}

Skinning::iterator Skinning::end() {
	return m_weights.end();
}
}  // namespace anim
