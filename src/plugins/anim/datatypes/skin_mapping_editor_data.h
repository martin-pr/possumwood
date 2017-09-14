#pragma once

#include "skeleton.h"

namespace anim {

/// A simple data container for frame editor node - holds individual transformations for each bone.
class SkinMappingEditorData {
  public:
	void setSkeleton(const Skeleton& s);
	const Skeleton& skeleton() const;

	typedef std::vector<std::pair<int, int>>::const_iterator const_iterator;
	typedef std::vector<std::pair<int, int>>::iterator iterator;

	void add(int fromJoint, int toJoint);
	void erase(iterator i);
	void clear();

	std::pair<int, int>& operator[](std::size_t index);
	const std::pair<int, int>& operator[](std::size_t index) const;

	const_iterator begin() const;
	const_iterator end() const;

	iterator begin();
	iterator end();

	bool empty() const;
	std::size_t size() const;

	bool operator==(const SkinMappingEditorData& d) const;
	bool operator!=(const SkinMappingEditorData& d) const;

  protected:
  private:
  	// this should be only a list of bones!
	Skeleton m_skeleton;
	std::vector<std::pair<int, int>> m_mapping;
};
}

namespace possumwood {

template<>
struct Traits<anim::SkinMappingEditorData> {
	static IO<anim::SkinMappingEditorData> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 0.2, 0}};
	}
};

}
