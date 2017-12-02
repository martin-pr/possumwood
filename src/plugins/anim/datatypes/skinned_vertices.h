#pragma once

#include <vector>
#include <initializer_list>

#include <ImathVec.h>

namespace anim {

/// A simple storage class for skinned vertices.
/// Each vertex has a position, and an arbitrary number of skinning weights (bone-weight pairs).
class SkinnedVertices {
  public:
	SkinnedVertices();

	class Vertex {
	  public:
		Vertex(const Imath::V3f& pos,
		       const std::initializer_list<std::pair<std::size_t, float>>& weights =
		           std::initializer_list<std::pair<std::size_t, float>>());

		const Imath::V3f& pos() const;
		void setPos(const Imath::V3f& p);

		std::size_t size() const;
		const std::pair<std::size_t, float>& operator[](std::size_t weightIndex) const;

		typedef std::vector<std::pair<std::size_t, float>>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		typedef std::vector<std::pair<std::size_t, float>>::iterator iterator;
		iterator begin();
		iterator end();

	  private:
		void addWeight(const std::size_t& bone, const float& w);

		Imath::V3f m_pos;
		std::vector<std::pair<std::size_t, float>> m_weights;

		friend class SkinnedVertices;
	};

	Vertex& add(const Imath::V3f& pos,
	                  const std::initializer_list<std::pair<std::size_t, float>>& weights =
	                      std::initializer_list<std::pair<std::size_t, float>>());

	std::size_t size() const;
	Vertex& operator[](std::size_t index);
	const Vertex& operator[](std::size_t index) const;

	typedef std::vector<Vertex>::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;

	typedef std::vector<Vertex>::iterator iterator;
	iterator begin();
	iterator end();

	std::size_t boneCount() const;
	/// a slightly weird API to allow boneCount update
	void addVertexWeight(std::size_t vertexIndex, std::size_t boneIndex, float weight);

  protected:
  private:
	std::vector<Vertex> m_vertices;
	std::size_t m_boneCount;
};
}
