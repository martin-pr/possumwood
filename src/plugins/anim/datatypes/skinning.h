#pragma once

#include <vector>

namespace anim {

class Skinning {
  public:
  	struct Weight {
  		Weight(std::size_t b, float w) : bone(b), weight(w) {
  		}

  		std::size_t bone;
  		float weight;
  	};

	void addWeight(std::size_t bone, float weight);

	void normalize();
	void limitInfluenceCount(std::size_t count);

	bool empty() const;
	std::size_t size() const;

	typedef std::vector<Weight>::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;

	typedef std::vector<Weight>::iterator iterator;
	iterator begin();
	iterator end();

  private:
	std::vector<Weight> m_weights;
};
}
