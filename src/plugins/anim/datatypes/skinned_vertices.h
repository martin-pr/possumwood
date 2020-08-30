#pragma once

#include <ImathVec.h>

#include <initializer_list>
#include <vector>

#include "skinning.h"

namespace anim {

/// A simple storage class for skinned vertices.
/// Each vertex has a position, and an arbitrary number of skinning weights (bone-weight
/// pairs).
class SkinnedVertices {
  public:
	class Vertex {
	  public:
		Vertex(const Imath::V3f& pos, const Skinning& skin);

		const Imath::V3f& pos() const;
		void setPos(const Imath::V3f& p);

		const Skinning& skinning() const;
		void setSkinning(const Skinning& skin);

	  private:
		Imath::V3f m_pos;
		Skinning m_skinning;

		friend class SkinnedVertices;
	};

	Vertex& add(const Imath::V3f& pos, const Skinning& skin = Skinning());

	std::size_t size() const;
	Vertex& operator[](std::size_t index);
	const Vertex& operator[](std::size_t index) const;

	typedef std::vector<Vertex>::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;

	typedef std::vector<Vertex>::iterator iterator;
	iterator begin();
	iterator end();

  protected:
  private:
	std::vector<Vertex> m_vertices;
};
}  // namespace anim
