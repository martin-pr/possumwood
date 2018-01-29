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
		auto toErase = m_weights.begin();
		for(auto it = m_weights.begin(); it != m_weights.end(); ++it)
			if(toErase->weight > it->weight)
				toErase = it;

		m_weights.erase(toErase);
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
}
