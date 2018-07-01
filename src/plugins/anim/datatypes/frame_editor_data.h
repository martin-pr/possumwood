#pragma once

#include "skeleton.h"

namespace anim {

/// A simple data container for frame editor node - holds individual transformations for each bone.
class FrameEditorData {
  public:
	void setSkeleton(const Skeleton& s);
	const Skeleton& skeleton() const;

	void setTransform(std::size_t joint, const Transform& tr);
	void resetTransform(std::size_t joint);
	Transform transform(std::size_t joint) const;
	void clear();

	typedef std::map<std::size_t, Transform>::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;

	std::size_t size() const;

	bool operator==(const FrameEditorData& d) const;
	bool operator!=(const FrameEditorData& d) const;

  protected:
  private:
  	// this should be only a list of bones!
	Skeleton m_skeleton;
	std::map<std::size_t, Transform> m_transforms;
};

std::ostream& operator << (std::ostream& out, const FrameEditorData& d);

}

namespace possumwood {

template<>
struct Traits<anim::FrameEditorData> {
	static IO<anim::FrameEditorData> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 0.2, 0}};
	}
};

}
